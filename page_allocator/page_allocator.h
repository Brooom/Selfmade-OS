#ifndef PAGE_ALLOCATOR_H
#define PAGE_ALLOCATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/**
 * @file page_allocator.h
 *
 * This allocator guarantees that no memory is allocated multiple times. It does not map the memory. The mapping is done by the kernel allocator.
 */

void init_allocator();
void* palloc(size_t size);
void pfree(void* pointer);
void* palloc_continuous_block(size_t nr_of_pages);

/**Soon gona be deleted. Should not be pushed */
void debug_helper_set_current_free(uint64_t new_current_free);
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif