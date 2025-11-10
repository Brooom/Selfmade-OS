#ifndef UART_H
#define UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "std/formating.h"



void uart_send(char *string, int string_length, ...);


/**
 * @brief Write string over uart
 *
 * Write a string to uart.
 *
 * @param *s Pointer to string
 * @param new_line 1 to append "\\n" after the string; 0 to omit it
 */
static void put_uart(const char *s, bool new_line);



//#############Legacy code
/** Send a hexvalue stored at a pointer p over uart
 * 
 * @param *p pointer to hex value
 * @param size size of the heyvalue 
 */
void uart_send_hex_value(volatile void *p, size_t size);

/** Send a int value over uart
 * 
 * @param v int value to send
 */
void uart_send_dec_value(int v);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif