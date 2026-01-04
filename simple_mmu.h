#include "allocator/allocator.h"

#define PAGESIZE 0x1000

#define PAGETABLEATTRIBUTES ((0b00000ULL<<59) | 0b11)
// simple_mmu.h
#define BLOCKATTRIBUTESFORNORMALMEMORY  0b11100000001           // AF=1, SH=Inner, AP=00, AttrIndx=0
#define BLOCKATTRIBUTESFORDEVICEMEMORY ((0b11ULL<<54) | 0b11100000101) // +XN, AttrIndx=1

#define INVALID 0b0

#define SCTLR_EL1 0b1000000011111
#define BLOCK_SIZE_L1 (1ULL << 31)
#define BLOCK_SIZE_L2 (1ULL << 21)


void init_pagetable();