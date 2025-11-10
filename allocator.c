#include "allocator.h"
#include <stdint.h>
#include <stddef.h>

extern uint8_t heap_start[];
uint8_t *current_free;
void init(){
    current_free = heap_start;
}

uint8_t* alloc(uint32_t size, uint32_t allignment){   
    current_free = (uint8_t*) allign(current_free, allignment);
    uint8_t *start_add = current_free;
    for (uint32_t i = 0; i < size; ++i){
        current_free[i] = 0;
    }
    current_free += size;
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