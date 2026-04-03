#include "kernel_allocator.h"

#include "../mmu/mmu.h"
#include "../page_allocator/page_allocator.h"
#include "../kernel_logs/kernel_logger_c_api.h"


static void* allign_addr(void *addr, int allignment);
static size_t allign(size_t size);

void init_kernel_allocator(){
    mmu_verbose(0);
    init_mmu();
}

void* kalloc(size_t size){
    uint64_t alligned_size = allign(size);
    void *adr = palloc(alligned_size);
    map_addr((uint64_t)adr, alligned_size);
    uint8_t* clear_ptr = (uint8_t*)adr;
    for(uint64_t i = 0; i < alligned_size; ++i){
        clear_ptr[i] = 0;
    }
    return adr;
}

void kfree(void* pointer){

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

