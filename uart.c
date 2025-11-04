#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "uart.h"

//address where to write the char values to for to transmit over uart
volatile unsigned int * const UART0DR = (unsigned int *)0x9000000;

void put_uart(const char *s, bool new_line) {
    while(*s != '\0') { /* Loop until end of string */
        *UART0DR = (unsigned int)(*s); /* Transmit char */
        s++; /* Next char */
    }
    if(new_line == 1){
        *UART0DR = (unsigned int)('\n');
    }
}

void uart_send_hex_value(volatile void *p, size_t size){
    char message[19];
    size *= 8;
    if(size==8){
        u8_to_hex(*(uint8_t*)p, message);
        put_uart(message, 1);
    }
    else if(size==16){
        u16_to_hex(*(uint16_t*)p, message);
        put_uart(message, 1);
    }
    else if(size==32){
        u32_to_hex(*(uint32_t*)p, message);
        put_uart(message, 1);
    }
    else if(size==64){
        u64_to_hex(*(uint64_t*)p, message);
        put_uart(message, 1);
    }
    else{
        put_uart("Can not convert message of size: ", 1);
        int_to_str(size, message);
        put_uart(message, 1);

    }
}

void uart_send_dec_value(int v){
    char message[11];
    int_to_str(v, message);
    put_uart(message, 1);
    
}

void u64_to_hex(uint64_t value, char *buf) {
    const char hex[] = "0123456789abcdef";

    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 16; ++i) {
        int shift = 60- (i * 4);
        buf[2 + i] = hex[(value >> shift) & 0xF];
    }
    buf[18] = '\0';
}

void u32_to_hex(uint32_t value, char *buf) {
    const char hex[] = "0123456789abcdef";

    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 8; ++i) {
        int shift = 28 - (i * 4);
        buf[2 + i] = hex[(value >> shift) & 0xF];
    }
    buf[10] = '\0';
}

void u16_to_hex(uint16_t value, char *buf) {
    const char hex[] = "0123456789abcdef";

    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 4; ++i) {
        int shift = 12 - (i * 4);
        buf[2 + i] = hex[(value >> shift) & 0xF];
    }
    buf[6] = '\0';
}

void u8_to_hex(uint8_t value, char *buf) {
    const char hex[] = "0123456789abcdef";

    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 2; ++i) {
        int shift = 4 - (i * 4);
        buf[2 + i] = hex[(value >> shift) & 0xF];
    }
    buf[4] = '\0';
}

void int_to_str(int value, char *buf) {
    char tmp[10];
    int len = 0;

    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    while (value > 0) {          // collect digits in reverse
        tmp[len++] = '0' + (value % 10);
        value /= 10;
    }

    for (int i = 0; i < len; ++i) // reverse into output buffer
        buf[i] = tmp[len - 1 - i];
    buf[len] = '\0';
}