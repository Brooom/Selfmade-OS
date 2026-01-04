#include "allocator/allocator.h"
#include <stdint.h>
#include <stddef.h>
#include "allocator.h"
#include "kernel_logs/kernel_logger_c_api.h"
#include "init_simple_mmu.S"
#include "simple_mmu.h"

struct mem{
    struct mem *next;
    unsigned int size;
};

extern uint8_t kernel_heap_start[];
extern uint8_t kernel_heap_end[];

struct mem *list_free = (struct mem *) kernel_heap_start;
struct mem *list_alloc = NULL;
int mem_size = sizeof(struct mem);

void init_mmu(){
    init_simple_mmu();
    *list_free = (struct mem){
        .next = NULL,
        .size = kernel_heap_end - kernel_heap_start-mem_size
    };
    list_alloc = NULL;
}


void* kalloc(int size, int allignment){
    struct mem *block_before = list_free;
    struct mem *block = list_free;
    while(block != NULL && block->size < (size+allignment-1+mem_size) ){
        block_before = block;
        block = block->next;
    }
    if(block == NULL){
        kernel_logger_log("No free memory block left for this size and this allignment."); // TODO: Throw error
        return NULL;
    }

    if(10*block->size < 11*(size+allignment-1+mem_size)){
        //remove from free list
        block_before->next = block->next;
        //address from where the memory of the given size is allocated
        void *pointer = allign((void*) (block+1), allignment);
        //add memory block to list of allocated elements
        block->next = list_alloc;
        list_alloc = block;
        return pointer;
    }
    else{
        block->size = block->size - (size+allignment-1+mem_size);
        struct mem *new_block = (struct mem *)((uint8_t *)(block + 1) + block->size);
        void *pointer = allign((void*) (new_block+1), allignment);
        new_block->next = list_alloc;
        new_block->size = size+allignment-1;
        list_alloc = new_block;
        return pointer;

    }
}

void kfree(void* pointer){

}


static void* allign(void *addr, int allignment)
{
    uintptr_t pointer = (uintptr_t) addr;
    pointer += allignment-1;
    pointer &= ~(allignment-1);
    return (void*) pointer;
}
