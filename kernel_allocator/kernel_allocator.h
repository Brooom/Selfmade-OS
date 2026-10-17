#ifndef KERNEL_ALLOCATOR_H
#define KERNEL_ALLOCATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

void init_kernel_allocator();
void init_kernel_allocator_verbose();

/**
 * @brief Allocate and map memory
 * 
 * Calls the page allocator to get a range of free addresses and calls the mmu to also map this addresses if not already mapped.
 */
void* kalloc(size_t size);
void kfree(void* pointer);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif