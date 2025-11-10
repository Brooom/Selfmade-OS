#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "uart.h"

//address where to write the char values to for to transmit over uart
volatile unsigned int * const UART0DR = (unsigned int *)0x9000000;

void uart_send(const char *string, int string_length, ...)
{
    char output_string[string_length];
    va_list args;
    va_start(args, string_length);
    format_string(string, output_string, string_length, args);
    va_end(args);
    put_uart(output_string, 1);
}

static void put_uart(const char *s, bool new_line)
{
    while(*s != '\0') { /* Loop until end of string */
        *UART0DR = (unsigned int)(*s); /* Transmit char */
        s++; /* Next char */
    }
    if(new_line == 1){
        *UART0DR = (unsigned int)('\n');
    }
}