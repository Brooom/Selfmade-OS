#include "allocator/allocator.h"
#include <stdint.h>
#include <stddef.h>
#include "kernel_logs/kernel_logger_c_api.h"
#include "simple_mmu.h"

extern uint8_t kernel_heap_start[];
extern uint8_t kernel_heap_end[];
extern void init_simple_mmu(void);
uint8_t *current_free = kernel_heap_start;

void init_mmu(){
    init_simple_mmu();
    init_pagetable();
}

void* kalloc(size_t size, int allignment){   
    current_free = (uint8_t*) allign(current_free, allignment);
    uint8_t *start_add = current_free;
    for (int i = 0; i < size; ++i){
        current_free[i] = 0;
    }
    current_free += size;
    return start_add;
}

void kfree(void *pointer)
{
    kernel_logger_log("The bump allocator can not free memory.");
}

static void* allign(void *addr, int allignment)
{
    uintptr_t pointer = (uintptr_t) addr;
    pointer += allignment-1;
    pointer &= ~(allignment-1);
    return (void *) pointer;
}

// static void *memcpy(void *dest, const void *src, size_t n) {
//     unsigned char *d = dest;
//     const unsigned char *s = src;

//     while (n--) {
//         *d++ = *s++;
//     }
//     return dest;
// }