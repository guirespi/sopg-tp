/************************************************************************************************
Copyright (c) 2024, Guido Ramirez <guidoramirez7@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

SPDX-License-Identifier: MIT
*************************************************************************************************/

/** @file dict_server.c
 ** @brief Dictionary server function implementation.
 **/

/* === Headers files inclusions =============================================================== */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include "dict_server.h"

/* === Macros definitions ====================================================================== */

#define SERVER_IP                "127.0.0.1"
#define SERVER_PORT              (5000)
#define SERVER_CLIENTS           (1)
#define SERVER_BUFFER_SIZE       (128)

#define SERVER_MAX_ARGS          (2) /**< Only two because the SET operation requires key:value. */

#define SERVER_GET_OP_STRING     "GET"
#define SERVER_SET_OP_STRING     "SET"
#define SERVER_DEL_OP_STRING     "DEL"

#define SERVER_OK_RESPONSE       "OK\n"
#define SERVER_NOTFOUND_RESPONSE "NOTFOUND\n"

#define LOG_INFO(format, ...)    printf("INFO-> " format "\n", ##__VA_ARGS__)
#define LOG_ERROR(format, ...)   fprintf(stderr, "ERROR -> " format "\n", ##__VA_ARGS__)

/* === Private data type declarations ========================================================== */

typedef enum {
    SERVER_OP_NONE = 0, /**< No operation */
    SERVER_OP_SET,      /**< Set key */
    SERVER_OP_GET,      /**< Get key */
    SERVER_OP_DEL,      /**< Delete key */
} server_op;

typedef enum {
    SERVER_OK = 0,
    SERVER_E_OS,
    SERVER_E_NULL,
    SERVER_E_SIZE,
    SERVER_E_BUFFER,
    SERVER_E_INVALID,
    SERVER_E_MISSING,
    SERVER_E_TOO_MANY,
    SERVER_E_NOT_FOUND,
} server_err_t;

typedef struct {
    server_op op;                 /**< Operation enum */
    char * args[SERVER_MAX_ARGS]; /**< Max arguments for all server's operations */
} server_op_t;

struct dict_server {
    int client_fd;         /**< Client file descriptor */
    int server_fd;         /**< Server file descriptor */
    server_op_t server_op; /**< Last server operation register */
};

/* === Private variable declarations =========================================================== */

/* === Private function declarations =========================================================== */

static int server_op_check(char * buffer, int length, server_op_t * digest);

static int server_write_key_value(server_op_t * digest);

static int server_read_key_value(server_op_t * digest, char * buffer, int buffer_size);

static int server_delete_key_value(server_op_t * digest);

static int server_op_process(int socket, server_op_t * digest);

/* === Public variable definitions ============================================================= */

/* === Private variable definitions ============================================================ */

/* === Private function implementation ========================================================= */
/**
 * @brief Check if an input buffer has the format for this server app.
 *
 * @param buffer Buffer to check.
 * @param length Buffer's length.
 * @param digest Result of operation check.
 * @return int
 *              - SERVER_OK if no error.
 */
static int server_op_check(char * buffer, int length, server_op_t * digest) {
    if (length < sizeof(SERVER_GET_OP_STRING)) // Just the minimum size of an operation. The three
                                               // availables are the same size.
        return SERVER_E_SIZE;
    if (buffer == NULL || length == 0)
        return SERVER_E_NULL;
    if (digest == NULL)
        return SERVER_E_NULL;

    int err = SERVER_OK;
    int op_args = 0;
    char * op = NULL;
    memset(digest, 0, sizeof(*digest));

    // Check if operation exists in the buffer.
    if (digest->op == SERVER_OP_NONE) {
        op = strstr(buffer, SERVER_GET_OP_STRING);
        if (op != NULL)
            digest->op = SERVER_OP_GET;
    }

    if (digest->op == SERVER_OP_NONE) {
        op = strstr(buffer, SERVER_SET_OP_STRING);
        if (op != NULL)
            digest->op = SERVER_OP_SET;
    }

    if (digest->op == SERVER_OP_NONE) {
        op = strstr(buffer, SERVER_DEL_OP_STRING);
        if (op != NULL)
            digest->op = SERVER_OP_DEL;
    }

    // Unknown operation.
    if (digest->op == SERVER_OP_NONE)
        return SERVER_E_INVALID;

    char * token = NULL;
    char * temp = buffer;
    const char * delim = " \n";

    // First toke is always the operation which was detected before.
    token = strtok_r(temp, delim, &temp);

    while ((token = strtok_r(temp, delim, &temp))) {
        if (op_args > SERVER_MAX_ARGS)
            return SERVER_E_TOO_MANY;
        digest->args[op_args] = token;
        op_args++;
    }

    // If two arguments were no received, error.
    if (digest->op == SERVER_OP_SET && op_args != SERVER_MAX_ARGS)
        return SERVER_E_MISSING;

    // For this operations we need one argument at least.
    if ((digest->op == SERVER_OP_GET || digest->op == SERVER_OP_DEL) && op_args != 1)
        return SERVER_E_MISSING;

    return SERVER_OK;
}
/**
 * @brief Write a key value.
 *
 * @param digest Result of previous operation format check.
 * @return int
 *              - SERVER_OK if no error.
 */
static int server_write_key_value(server_op_t * digest) {
    if (digest == NULL)
        return SERVER_E_NULL;

    int fd;
    int cnt;
    int err = SERVER_OK;

    // Our first argument is the file's name.
    fd = open(digest->args[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        LOG_ERROR("Can not open file [%s] to write key", digest->args[0]);
        return SERVER_E_OS;
    }

    cnt = write(fd, digest->args[1], strlen(digest->args[1]));
    if (cnt == 0) {
        err = SERVER_E_OS;
        goto finish;
    }

finish:
    close(fd);
    return err;
}
/**
 * @brief Read a key value.
 *
 * @param digest Result of previous operation format check.
 * @param buffer Buffer where the reading will be stored.
 * @param buffer_size Buffer's size.
 * @return int
 *              - SERVER_OK if not error.
 *              - SERVER_E_NOTFOUND if the key does not exist.
 */
static int server_read_key_value(server_op_t * digest, char * buffer, int buffer_size) {
    if (digest == NULL)
        return SERVER_E_NULL;

    int fd;
    int cnt;
    int err = SERVER_OK;

    // Our first argument is the file's name.
    fd = open(digest->args[0], O_RDONLY);
    if (fd < 0) {
        LOG_ERROR("Can not open file [%s] to read key", digest->args[0]);
        return SERVER_E_NOT_FOUND;
    }

    cnt = read(fd, buffer, buffer_size);
    if (cnt == 0) {
        err = SERVER_E_NOT_FOUND;
        goto finish;
    }

    LOG_INFO("Read %d byte from [%s] file", cnt, digest->args[0]);

finish:
    close(fd);
    return err;
}
/**
 * @brief Delete a key value.
 *
 * @param digest Result of previous operation format check.
 * @return int
 *              - SERVER_OK if no error.
 */
static int server_delete_key_value(server_op_t * digest) {
    if (digest == NULL)
        return SERVER_E_NULL;

    int status;
    int cnt;
    int err = SERVER_OK;

    status = remove(digest->args[0]);

    if (status) {
        LOG_ERROR("Can not delete [%s] file", digest->args[0]);
        err = SERVER_E_NOT_FOUND;
    }

    return err;
}
/**
 * @brief Process and responds to a previous operation format check.
 *
 * @param socket Socket to send the response to.
 * @param digest Result of previous operation format check.
 * @return int
 *              - SERVER_OK if no error.
 */
static int server_op_process(int socket, server_op_t * digest) {
    if (digest == NULL)
        return SERVER_E_NULL;

    int err = SERVER_OK;
    char buffer[128] = {0};

    if (digest->op == SERVER_OP_SET) {
        err = server_write_key_value(digest);
    } else if (digest->op == SERVER_OP_GET) {
        err = server_read_key_value(digest, buffer, sizeof(buffer));
    } else if (digest->op == SERVER_OP_DEL) {
        err = server_delete_key_value(digest);
    } else {
        err = SERVER_E_NOT_FOUND;
    }

    if (err == SERVER_OK) {
        // Send response.
        int rt;
        rt = send(socket, SERVER_OK_RESPONSE, sizeof(SERVER_OK_RESPONSE), MSG_DONTWAIT);
        if (rt <= 0) {
            LOG_ERROR("Error sending OK response");
            err = SERVER_E_OS;
        }

        if (digest->op == SERVER_OP_GET && rt > 0) {
            strcat(buffer, "\n");
            rt = send(socket, buffer, strlen(buffer), MSG_DONTWAIT);
            if (rt <= 0) {
                LOG_ERROR("Error sending GET response");
                err = SERVER_E_OS;
            }
        }
    } else {
        // Send error message.
        if (err == SERVER_E_NOT_FOUND) {
            int rt;
            rt = send(socket, SERVER_NOTFOUND_RESPONSE, sizeof(SERVER_NOTFOUND_RESPONSE),
                      MSG_DONTWAIT);
            if (rt <= 0) {
                LOG_ERROR("Error sending NOTFOUND response");
                err = SERVER_E_OS;
            }
        } else {
            int rt;
            sprintf(buffer, "ERROR:%d", err);
            rt = send(socket, buffer, strlen(buffer), MSG_DONTWAIT);
            if (rt <= 0) {
                LOG_ERROR("Error sending ERROR response");
                err = SERVER_E_OS;
            }
        }
    }
    return err;
}

/* === Public function implementation ========================================================== */

dict_server dict_server_init(void) {
    dict_server server = malloc(sizeof(*server));
    return server;
}

int dict_server_start(void) {
    // Create a server socket.
    int s = socket(AF_INET, SOCK_STREAM, 0);

    // Set REUSEADDR to server's socket.
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        LOG_ERROR("setsockopt REUSEADDR failed");
        exit(EXIT_FAILURE);
    }

    // Load [ip:port] to server.
    struct sockaddr_in serveraddr;
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &(serveraddr.sin_addr)) <= 0) {
        LOG_ERROR("Invalid IP address");
        exit(EXIT_FAILURE);
    }

    // Open port with bind().
    if (bind(s, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
        LOG_ERROR("Bind");
        exit(EXIT_FAILURE);
    }

    // Socket in listening mode.
    if (listen(s, SERVER_CLIENTS) == -1) { // backlog=10
        LOG_ERROR("Listen");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        // Receive new connections.
        socklen_t addr_len = sizeof(struct sockaddr_in);
        struct sockaddr_in clientaddr;
        int newfd;
        LOG_INFO("Server : Waiting for connection...");
        if ((newfd = accept(s, (struct sockaddr *)&clientaddr, &addr_len)) == -1) {
            LOG_ERROR("Accept");
            exit(EXIT_FAILURE);
        }

        char ipClient[32];
        inet_ntop(AF_INET, &(clientaddr.sin_addr), ipClient, sizeof(ipClient));
        LOG_INFO("Server : Connection from  [%s]", ipClient);

        // Read the client's message.
        int len = 0;
        char buffer[SERVER_BUFFER_SIZE] = {0};

        while (len = recv(newfd, buffer, SERVER_BUFFER_SIZE, MSG_DONTWAIT)) {
            if (len < 0) {
                switch (errno) {
                case ENOTCONN:
                    LOG_ERROR("Client is disconnecting...");
                    break;
                case ECONNRESET:
                    LOG_ERROR("Peer reset connection...");
                    break;
                default:
                    break;
                }
            } else if (len > 0) {
                buffer[len] = 0;
                server_op_t digest = {0};
                LOG_INFO("%d bytes arrived into server: %s", len, buffer);
                int err = server_op_check(buffer, len, &digest);
                if (err != 0) {
                    LOG_ERROR("Can not check input data. Returned [%d]", err);
                } else {
                    err = server_op_process(newfd, &digest);
                    LOG_INFO("Server process finished. Returned [%d]", err);
                }
            }
        }
        // Close connection with client.
        close(newfd);
    }

    return EXIT_SUCCESS;
}

/* === End of documentation ==================================================================== */