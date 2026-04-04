#include "page_allocator/page_allocator.h"
#include <stdint.h>
#include <stddef.h>
#include "kernel_logs/kernel_logger_c_api.h"
#include "../mmu/mmu.h"

extern uint8_t kernel_heap_start[];
extern uint8_t kernel_heap_end[];
uint8_t *current_free = kernel_heap_start;

static void* allign_addr(void *addr, int allignment);
static size_t allign(size_t size);

void init_page_allocator();


void* palloc(size_t size){
    current_free = (uint8_t*) allign_addr(current_free, PAGESIZE);
    size_t alligned_size = allign(size);
    uint8_t *start_add = current_free;
    current_free += alligned_size;
    return start_add;
}
/** Returns a block of memory that is coninous and in the l3 pagetable.
 * This function is needed for page table allocation.
 * 
*/
void* palloc_continuous_block(size_t nr_of_pages){
    current_free = (uint8_t*) allign_addr(current_free, PAGESIZE);
    uint8_t *start_add = current_free;
    size_t pages_left = (BLOCK_SIZE_L2 - ((uintptr_t)current_free & (BLOCK_SIZE_L2 - 1)));
    
    
    if(nr_of_pages>4096){
        kernel_logger_log("Error: A continous block in the same l3 pagetable can not be bigger than 4096.");
    }
    else if(pages_left >= nr_of_pages){
        current_free += nr_of_pages*4096;
    }
    else{
        start_add = (uint8_t*)(((uintptr_t)current_free + BLOCK_SIZE_L2) & ~(BLOCK_SIZE_L2 - 1));
        current_free = start_add + nr_of_pages*4096;
    }
    return start_add;
}

void pfree(void *pointer)
{
    kernel_logger_log("The bump allocator can not free memory.");
}

static void* allign_addr(void *addr, int allignment)
{
    uintptr_t pointer = (uintptr_t) addr;
    pointer += allignment-1;
    pointer &= ~(allignment-1);
    return (void *) pointer;
}
static size_t allign(size_t size){
    int over_allignment = size%4096;
    if(over_allignment > 0){
        return size + 4096-(over_allignment);
    }
    return size;
}

// static void *memcpy(void *dest, const void *src, size_t n) {
//     unsigned char *d = dest;
//     const unsigned char *s = src;

//     while (n--) {
//         *d++ = *s++;
//     }
//     return dest;
// }


void debug_helper_set_current_free(uint64_t new_current_free){
    current_free = (uint8_t*)new_current_free;
}