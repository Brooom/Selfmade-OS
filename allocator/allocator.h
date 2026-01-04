#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

void init_mmu();
void* kalloc(size_t size, int allignment);
void kfree(void* pointer);
static void* allign(void *addr, int allignment);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif