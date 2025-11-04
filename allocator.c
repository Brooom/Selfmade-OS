#include "allocator.h"
#include <stdint.h>
#include "uart.h"
#include <stddef.h>

extern uint8_t heap_start[];
uint8_t *current_free;
void init(){
    current_free = heap_start;
}

uint8_t* alloc(uint32_t size, uint32_t allignment){   
    put_uart("Current free:  ", 0);
    uart_send_hex_value(&current_free, sizeof(current_free));
    current_free = (uint8_t*) allign(current_free, allignment);
    put_uart("Byte aligned:  ", 0);
    uart_send_hex_value(&current_free, sizeof(current_free));
    put_uart("Size: ", 0);
    uart_send_hex_value(&size, sizeof(size));
    uint8_t *start_add = current_free;
    for (uint32_t i = 0; i < size; ++i){
        current_free[i] = 0;
    }
    current_free += size;

    put_uart("Storage added: ", 0);
    uart_send_hex_value(&current_free, sizeof(current_free));
    return start_add;
}

uintptr_t allign(void *addr, uint32_t allignment){
    uintptr_t pointer = (uintptr_t) addr;
    pointer += allignment-1;
    pointer &= ~(allignment-1);
    return pointer;
}


void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;

    while (n--) {
        *d++ = *s++;
    }
    return dest;
}