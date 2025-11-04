#ifndef UART_H
#define UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/**
 * @brief Write string over uart
 *
 * Write a string to uart.
 *
 * @param *s Pointer to string
 * @param new_line 1 to append "\\n" after the string; 0 to omit it
 */
void put_uart(const char *s, bool new_line);

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

/** Transform uint64_t value int a sting*/
void u64_to_hex(uint64_t value, char *buf);
/** Transform uint32_t value int a sting*/
void u32_to_hex(uint32_t value, char *buf);
/** Transform uint16_t value int a sting*/
void u16_to_hex(uint16_t value, char *buf);
/** Transform uint8_t value int a sting*/
void u8_to_hex(uint8_t value, char *buf);
/** Transform an int value to a string.*/
void int_to_str(int value, char *buf);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif