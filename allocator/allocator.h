#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

void init();
void* kalloc(uint32_t size, uint32_t allignment);
void kfree(void* pointer);
static uintptr_t allign(void *addr, uint32_t allignment);
static void *memcpy(void *dest, const void *src, size_t n);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif