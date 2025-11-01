#include <stdbool.h>

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
