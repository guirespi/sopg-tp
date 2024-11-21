/************************************************************************************************
Copyright (c) 2023, Guido Ramirez <guidoramirez7@gmail.com>

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

#ifndef GPIO_H
#define GPIO_H

/** @file gpio.h
 ** @brief GPIO public header.
 **/

/* === Headers files inclusions ================================================================ */

#include <stdint.h>
#include <stdbool.h>

/* === C++ header ============================================================================ */

#ifdef __cplusplus
extern "C" {
#endif

/* === Public macros definitions =============================================================== */

/* === Public data type declarations =========================================================== */

/**
 * @brief GPIO instance. It contains relevant hardware and software information for the internal
 * implementation.
 *
 */
typedef struct gpio_s * gpio_t;

/* === Public variable declarations ============================================================ */

/* === Public function declarations ============================================================ */

/**
 * @brief Create a GPIO instance.
 *
 * @param port GPIO port.
 * @param bit GPIO bit.
 * @return gpio_t Return GPIO instance.
 *              - NULL value if error.
 */
gpio_t gpioCreate(uint8_t port, uint8_t bit);
/**
 * @brief Set GPIO output.
 *
 * @param gpio GPIO instance.
 * @param output Output to set.
 */
void gpioSetOutput(gpio_t gpio, bool output);
/**
 * @brief Set GPIO state.
 *
 * @param gpio GPIO instance.
 * @param state GPIO state to set.
 */
void gpioSetState(gpio_t gpio, bool state);
/**
 * @brief Get GPIO state.
 *
 * @param gpio GPIO instance.
 * @return true GPIO is set.
 * @return false GPIO is not set.
 */
bool gpioGetState(gpio_t gpio);

/* === End of documentation ==================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* GPIO_H */