#ifndef FROMATING_H
#define FORMATING_H

#include <stdint.h>
#include <stddef.h>
#include "args.h"

#ifdef __cplusplus
extern "C" {
#endif

void format_string(const char * input, char *output, int max_output_size, va_list va_list);

/** Transform int of different sizes to a hex string*/
static void int_to_hex_string(void *p, size_t size, char *buffer);
/** Transform uint64_t value int a sting*/
static void u64_to_hex(uint64_t value, char *buf);
/** Transform uint32_t value int a sting*/
static void u32_to_hex(uint32_t value, char *buf);
/** Transform uint16_t value int a sting*/
static void u16_to_hex(uint16_t value, char *buf);
/** Transform uint8_t value int a sting*/
static void u8_to_hex(uint8_t value, char *buf);
/** Transform an int value to a string.*/
static void int_to_str(int value, char *buf);

#ifdef __cplusplus
}
#endif

#endif
