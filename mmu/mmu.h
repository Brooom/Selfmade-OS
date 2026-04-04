#include "../page_allocator/page_allocator.h"

#define PAGESIZE 0x1000

#define PAGETABLEATTRIBUTES ((0b00000ULL<<59) | 0b11)

#define PAGEATTRIBUTESFORNORMALMEMORY  0b11100000011   
#define PAGEATTRIBUTESFORDEVICEMEMORY  ((0b11ULL<<53) | 0b11100000111) 

#define BLOCKATTRIBUTESFORNORMALMEMORY  0b11100000001
#define BLOCKATTRIBUTESFORDEVICEMEMORY ((0b11ULL<<53) | 0b11100000101) 

#define INVALID 0b0

#define SCTLR_EL1 0b1000000011111

#define BLOCK_SIZE_SHIFT_L0 39
#define BLOCK_SIZE_L0 (1ULL << BLOCK_SIZE_SHIFT_L0)
#define BLOCK_SIZE_SHIFT_L1 30
#define BLOCK_SIZE_L1 (1ULL << BLOCK_SIZE_SHIFT_L1)
#define BLOCK_SIZE_SHIFT_L2 21
#define BLOCK_SIZE_L2 (1ULL << BLOCK_SIZE_SHIFT_L2)
#define BLOCK_SIZE_SHIFT_L3 12
#define BLOCK_SIZE_L3 (1ULL << BLOCK_SIZE_SHIFT_L3)

#define BASE_ADDRESS 0x40000000
#define NORMAL_MEMORY_SIZE 0x10000000


struct map_new_page_tables;


/**Initialize root of translation table*/
void init_mmu();

/**Map a specified range of memory */
void map_addr(uint64_t address, uint64_t size);

/**Unmap a specified range of memory */
void unmap_addr();

/**Map device memory */
void register_device_memory(uintptr_t address, uint32_t size);


/**Enable logging in the mmu */
void mmu_verbose(int enable);