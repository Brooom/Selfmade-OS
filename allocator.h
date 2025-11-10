#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

void init();
uint8_t* alloc(uint32_t size, uint32_t allignment);
uintptr_t allign(void *addr, uint32_t allignment);
void *memcpy(void *dest, const void *src, size_t n);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif