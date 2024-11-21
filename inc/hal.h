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

#ifndef HAL_H
#define HAL_H

/** @file hal.h
 ** @brief HAL public header.
 **/

/* === Headers files inclusions ================================================================ */

#include <stdbool.h>
#include <stdint.h>

/* === C++ header ============================================================================ */

#ifdef __cplusplus
extern "C" {
#endif

/* === Public macros definitions =============================================================== */

/* === Public data type declarations =========================================================== */

/* === Public variable declarations ============================================================ */

/* === Public function declarations ============================================================ */

/**
 * @brief Set GPIO direction.
 *
 * @param port GPIO port.
 * @param bit GPIO bit.
 * @param output GPIO output.
 */
void hal_gpio_set_direction(uint8_t port, uint8_t bit, bool output);
/**
 * @brief Set the GPIO output.
 *
 * @param port GPIO port.
 * @param bit GPIO bit.
 * @param active GPIO output state.
 */
void hal_gpio_set_output(uint8_t port, uint8_t bit, bool active);
/**
 * @brief Get GPIO input value.
 *
 * @param port GPIO port.
 * @param bit GPIO bit.
 * @return true GPIO input value is set.
 * @return false GPIO input value is reset.
 */
bool hal_gpio_get_input(uint8_t port, uint8_t bit);

/* === End of documentation ==================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* HAL_H */