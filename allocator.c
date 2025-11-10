#include "allocator.h"
#include <stdint.h>
#include "kernel_logs/kernel_logger_c_api.h"
#include <stddef.h>

extern uint8_t heap_start[];
uint8_t *current_free;
void init(){
    current_free = heap_start;
}

uint8_t* alloc(uint32_t size, uint32_t allignment){   
    kernel_logger_log("Current free: %x", current_free);
    current_free = (uint8_t*) allign(current_free, allignment);
    kernel_logger_log("Byte aligned: %x", current_free);
    kernel_logger_log("Size: %x", size);
    uint8_t *start_add = current_free;
    for (uint32_t i = 0; i < size; ++i){
        current_free[i] = 0;
    }
    current_free += size;

    kernel_logger_log("Storage added: %x", current_free);
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