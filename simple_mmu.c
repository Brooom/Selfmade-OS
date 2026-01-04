#include "simple_mmu.h"
#include "kernel_logs/kernel_logger_c_api.h"

void init_pagetable(){
    uint64_t *l0_table = (uint64_t*) kalloc(PAGESIZE, PAGESIZE);
    uint64_t *l1_table_0 = (uint64_t*) kalloc(PAGESIZE, PAGESIZE);
    uint64_t *l1_table_1 = (uint64_t*) kalloc(PAGESIZE, PAGESIZE);
    uint64_t *l2_table_0 = (uint64_t*) kalloc(PAGESIZE, PAGESIZE);
    uint64_t *l2_table_1 = (uint64_t*) kalloc(PAGESIZE, PAGESIZE);
    l0_table[0] = (int64_t)l1_table_0 | PAGETABLEATTRIBUTES;
    l0_table[1] = (int64_t)l1_table_1 | PAGETABLEATTRIBUTES;
    l1_table_0[0] = (int64_t)l2_table_0 | PAGETABLEATTRIBUTES;
    l1_table_0[1] = (int64_t)l2_table_1 | PAGETABLEATTRIBUTES;
    kernel_logger_log("Number of entries in the table: %u", (int)(PAGESIZE/8));
    for(int i = 0; i<(int)(PAGESIZE/8); ++i){
        l2_table_0[i] = BLOCKATTRIBUTESFORNORMALMEMORY | ((uint64_t)i * BLOCK_SIZE_L2);
    }

    for(int i = 0; i<(int)(PAGESIZE/8); ++i){
        l2_table_1[i] = BLOCKATTRIBUTESFORNORMALMEMORY | (0x40000000 + (uint64_t)i * BLOCK_SIZE_L2);
    }

    for(uint64_t i = 2; i<(int)(PAGESIZE/8); ++i){
        l1_table_0[i] = BLOCKATTRIBUTESFORDEVICEMEMORY | ((uint64_t)0x40000000 * i);
        //kernel_logger_log("Map address %p", ((uint64_t)0x40000000 * i));
    }
    for(uint64_t i = 0; i<(int)(PAGESIZE/8); ++i){
        l1_table_1[i] = BLOCKATTRIBUTESFORDEVICEMEMORY | ((uint64_t)0x8000000000 + (uint64_t)0x40000000 * i);
        kernel_logger_log("Map address %p", (0x8000000000 + (uint64_t)0x40000000 * i));
    }

    kernel_logger_log("%p", l0_table);
    asm volatile("msr ttbr0_el1, %0\n"
                 "isb"
                 :
                 : "r"((uint64_t)l0_table)
                 : "memory");

    asm volatile("msr sctlr_el1, %0\n"
                 "isb"
                 :
                 : "r"(SCTLR_EL1)
                 : "memory");
    int u = 1;
    int k = 1;
    kernel_logger_log("%u", u);
    kernel_logger_log("Finished MMU");
}