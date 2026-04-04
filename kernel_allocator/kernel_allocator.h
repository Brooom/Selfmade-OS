#ifndef KERNEL_ALLOCATOR_H
#define KERNEL_ALLOCATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

void init_kernel_allocator();
void* kalloc(size_t size);
void kfree(void* pointer);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif