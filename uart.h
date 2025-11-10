#ifndef UART_H
#define UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "std/formating.h"



void uart_send(const char *string, int string_length, ...);
/**
 * @brief Write string over uart
 *
 * Write a string to uart.
 *
 * @param *s Pointer to string
 * @param new_line 1 to append "\\n" after the string; 0 to omit it
 */
static void put_uart(const char *s, bool new_line);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif