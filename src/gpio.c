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

/** @file gpio.c
 ** @brief GPIO function implementations.
 **/

/* === Headers files inclusions =============================================================== */

#include <string.h>
#include "gpio.h"
#include "hal.h"

/* === Macros definitions ====================================================================== */

#ifndef MAX_GPIO_INSTANCES
/*< Max static GPIO instances */
#define MAX_GPIO_INSTANCES 10
#endif

/* === Private data type declarations ========================================================== */

struct gpio_s {
    uint8_t port; /**< GPIO port */
    uint8_t bit;  /**< GPIO bit */
    bool output;  /**< GPIO output */
#ifndef USE_DYNAMIC_MEM
    bool used; /**< For static allocation. Busy state of the instance */
#endif
};

/* === Private variable declarations =========================================================== */

/* === Private function declarations =========================================================== */

static gpio_t gpioAllocate(void);

/* === Public variable definitions ============================================================= */

/* === Private variable definitions ============================================================ */

/* === Private function implementation ========================================================= */

#ifndef USE_DYNAMIC_MEM
/**
 * @brief Static allocation of GPIO instance.
 *
 * @return gpio_t Return GPIO instance.
 *              - NULL if no static instance left.
 */
static gpio_t gpioAllocate(void) {
    static struct gpio_s instances[MAX_GPIO_INSTANCES] = {0};

    gpio_t self = NULL;
    for (int index = 0; index < MAX_GPIO_INSTANCES; index++) {
        if (!instances[index].used) {
            self = &instances[index];
            self->used = true;
            break;
        }
    }
    return self;
}
#endif

/* === Public function implementation ========================================================== */

gpio_t gpioCreate(uint8_t port, uint8_t bit) {
    gpio_t self;
#ifdef USE_DYNAMIC_MEM
    self = malloc(sizeof(struct gpio_h));
#else
    self = gpioAllocate();
#endif
    if (self) {
        self->port = port;
        self->bit = bit;
        self->output = false;
    }
    return self;
}

void gpioSetOutput(gpio_t self, bool output) {
    self->output = output;
    hal_gpio_set_direction(self->port, self->bit, output);
}

void gpioSetState(gpio_t self, bool state) {
    if (self->output) {
        hal_gpio_set_direction(self->port, self->bit, self->output);
    }
}

bool gpioGetState(gpio_t self) {
    return hal_gpio_get_input(self->port, self->bit);
}

/* === End of documentation ==================================================================== */