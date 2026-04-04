#include "mmu.h"
#include "../kernel_logs/kernel_logger_c_api.h"
#include "../uart_driver/uart.h"

extern char kernel_start[];
extern char kernel_end[];
extern char init_page_l0[];
extern char init_page_l1[];
extern char init_page_l2[];
extern char init_page_l3[];
extern char tmp_page_1[];
extern char tmp_page_2[];
extern char tmp_page_3[];
extern void init_mmu_first_step(void);

static int verbose;
static uint64_t *l0_table;
static uint64_t *tmp_l1_table;
static uint64_t *tmp_l2_table;
static uint64_t *tmp_l3_table;

struct map_new_page_tables{
    uint64_t* new_pagetable;
    uint64_t* new_l3_pagetable;
    uint64_t* new_l2_pagetable;
    uint64_t* new_l1_pagetable;
};

static uint64_t* phys_to_virt(uintptr_t phy_addr);
static uintptr_t virt_to_phys(uint64_t* phy_addr);
static uint64_t* map_page_for_pagetable();
static void map_page_for_table_h(uint64_t* parent_pagetable, struct map_new_page_tables page_to_maps, uint8_t level);
static void zero_out_page(uint64_t* page);
static void mmu_map_4kb(uint64_t* root_pagetable, uint64_t va, uint64_t pa, uint8_t attr_idx);
//void mmu_map_1gb(uint64_t* root_pagetable, uint64_t va, uint64_t pa, uint8_t attr_idx);
//void mmu_map_2mb(uint64_t* root_pagetable, uint64_t va, uint64_t pa, uint8_t attr_idx);

/**Initialize root of translation table*/
void init_mmu(){
    init_mmu_first_step();

    l0_table = (uint64_t*)init_page_l0;
    zero_out_page(l0_table);
    uint64_t l0_index = ((uint64_t) l0_table >> BLOCK_SIZE_SHIFT_L0) & 0x1FF; 
    uint64_t l1_index = ((uint64_t) l0_table >> BLOCK_SIZE_SHIFT_L1) & 0x1FF;
    uint64_t l2_index = ((uint64_t) l0_table >> BLOCK_SIZE_SHIFT_L2) & 0x1FF;
    uint64_t l3_index = ((uint64_t) l0_table >> BLOCK_SIZE_SHIFT_L3) & 0x1FF;

    uint64_t *l1_table = (uint64_t*)init_page_l1;
    zero_out_page(l1_table);
    l0_table[l0_index] = PAGETABLEATTRIBUTES | virt_to_phys(l1_table);

    uint64_t *l2_table = (uint64_t*)init_page_l2;
    zero_out_page(l2_table);
    l1_table[l1_index] = PAGETABLEATTRIBUTES | virt_to_phys(l2_table);

    uint64_t *l3_table = (uint64_t*)init_page_l3;
    zero_out_page(l3_table);
    l2_table[l2_index] = PAGETABLEATTRIBUTES | virt_to_phys(l3_table);

    l3_table[0] = PAGEATTRIBUTESFORNORMALMEMORY | virt_to_phys((uint64_t*)init_page_l0);
    l3_table[1] = PAGEATTRIBUTESFORNORMALMEMORY | virt_to_phys((uint64_t*)init_page_l1);
    l3_table[2] = PAGEATTRIBUTESFORNORMALMEMORY | virt_to_phys((uint64_t*)init_page_l2);
    l3_table[3] = PAGEATTRIBUTESFORNORMALMEMORY | virt_to_phys((uint64_t*)init_page_l3);
    l3_table[4] = PAGEATTRIBUTESFORNORMALMEMORY | virt_to_phys((uint64_t*)tmp_page_1);
    l3_table[5] = PAGEATTRIBUTESFORNORMALMEMORY | virt_to_phys((uint64_t*)tmp_page_2);
    l3_table[6] = PAGEATTRIBUTESFORNORMALMEMORY | virt_to_phys((uint64_t*)tmp_page_3);

    tmp_l1_table = (uint64_t*)tmp_page_1;
    tmp_l2_table = (uint64_t*)tmp_page_2;
    tmp_l3_table = (uint64_t*)tmp_page_3;

    uintptr_t start = (uintptr_t)&kernel_start;
    uintptr_t end   = (uintptr_t)&kernel_end;
    map_addr(start, end-start);
    init_uart();
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
    kernel_logger_log_no_terminal("3");
    int i = 3;
    if(verbose) kernel_logger_log_no_terminal("Finished MMU");
}


/**Map 1gb sized blocks (Not in use currently)*/
/* void mmu_map_1gb(uint64_t* root_pagetable, uintptr_t va, uintptr_t pa, uint8_t attr_idx){
    uint64_t l0_index = ((uint64_t)va >> BLOCK_SIZE_SHIFT_L0) & 0x1FF; 
    uint64_t l1_index = ((uint64_t)va >> BLOCK_SIZE_SHIFT_L1) & 0x1FF;

    //Check if index l0 is invalide. If yes initalize page table
    if(!(root_pagetable[l0_index] & 1)){
        if(verbose){
            kernel_logger_log("There is no an initialized page table at index l0: %u", l0_index);
        }
        uintptr_t l1 = (uintptr_t)palloc(PAGESIZE);
        uintptr_t l1_table = virt_to_phys((uint64_t*)l1);
        root_pagetable[l0_index] = (uint64_t)l1_table | PAGETABLEATTRIBUTES;
        if(verbose){
            kernel_logger_log("Map page table l1.");
        }
        mmu_map_4kb(root_pagetable, l1, l1, 0);
        
    }
    uintptr_t l1_phy_addr = root_pagetable[l0_index] & 0x0000FFFFFFFFF000ULL;
    uint64_t* l1 = phys_to_virt(l1_phy_addr);

    if(!(l1[l1_index] & 1)){
        if(verbose){
            kernel_logger_log("There is no initialized page table at index l1: %u", l1_index);
        }
        if(attr_idx==0){
            uintptr_t block = BLOCKATTRIBUTESFORNORMALMEMORY | pa;
            l1[l1_index] = (uint64_t)block;
            if(verbose){
                kernel_logger_log("Created block at idx0: %u, idx1: %u", l0_index, l1_index);
            }
        }
        else if(attr_idx == 1){
            uintptr_t block = BLOCKATTRIBUTESFORDEVICEMEMORY | pa;
            l1[l1_index] = (uint64_t)block;
            if(verbose){
                kernel_logger_log("Created block at idx0: %u, idx1: %u", l0_index, l1_index);
            }
        }
        else{
            kernel_logger_log("The memory attribute index %u does not exist.", attr_idx);
        }
    }
} */

/**Map 2mb sized blocks (Not in use currently)*/
/* void mmu_map_2mb(uint64_t* root_pagetable, uintptr_t va, uintptr_t pa, uint8_t attr_idx){
    uint64_t l0_index = ((uint64_t) va >> BLOCK_SIZE_SHIFT_L0) & 0x1FF; 
    uint64_t l1_index = ((uint64_t) va >> BLOCK_SIZE_SHIFT_L1) & 0x1FF;
    uint64_t l2_index = ((uint64_t) va >> BLOCK_SIZE_SHIFT_L2) & 0x1FF;

    //Check if index l0 is invalide. If yes initalize page table
    if(!(root_pagetable[l0_index] & 1)){
        if(verbose){
            kernel_logger_log("There is no an initialized page table at index l0: %u", l0_index);
        }
        uintptr_t l1 = (uintptr_t)palloc(PAGESIZE);
        uintptr_t l1_table = virt_to_phys((uint64_t*)l1);
        root_pagetable[l0_index] = (uint64_t)l1_table | PAGETABLEATTRIBUTES;
        if(verbose){
            kernel_logger_log("Map page table l1.");
        }
        mmu_map_4kb(root_pagetable, l1, l1, 0);
    }
    uintptr_t l1_phy_addr = root_pagetable[l0_index] & 0x0000FFFFFFFFF000ULL;
    uint64_t* l1 = phys_to_virt(l1_phy_addr);

    if(!(l1[l1_index] & 1)){
        if(verbose){
            kernel_logger_log("There is no initialized page table at index l1: %u", l1_index);
        }
        uintptr_t l2 = (uintptr_t)palloc(PAGESIZE);
        uintptr_t l2_table = virt_to_phys((uint64_t*)l2);
        l1[l1_index] = (uint64_t)l2_table | PAGETABLEATTRIBUTES;
        if(verbose){
            kernel_logger_log("Map page table l2.");
        }
        mmu_map_4kb(root_pagetable, l2, l2, 0);
    }
    uintptr_t l2_phy_addr = l1[l1_index] & 0x0000FFFFFFFFF000ULL;
    uint64_t* l2 = phys_to_virt(l2_phy_addr);

    if(!(l2[l2_index] & 1)){
        if(verbose){
            kernel_logger_log("There is no initialized page table at index l2: %u", l2_index);
        }
        if(attr_idx==0){
            uintptr_t block = BLOCKATTRIBUTESFORNORMALMEMORY | pa;
            l2[l2_index] = (uint64_t)block;
            if(verbose){
                kernel_logger_log("Created block at idx0: %u, idx1: %u, idx2: %u", l0_index, l1_index, l2_index);
            }
        }
        else if(attr_idx == 1){
            uint64_t block = BLOCKATTRIBUTESFORDEVICEMEMORY | pa;
            l2[l2_index] = (uint64_t)block;
            if(verbose){
                kernel_logger_log("Created block at idx0: %u, idx1: %u, idx2: %u", l0_index, l1_index, l2_index);
            }
        }
        else{
            kernel_logger_log("The memory attribute index %u does not exist.", attr_idx);
        }
    }
} */

/**Map 4kb sized blocks*/
static void mmu_map_4kb(uint64_t* root_pagetable, uintptr_t va, uintptr_t pa, uint8_t attr_idx){
    uint64_t l0_index = ((uint64_t) va >> BLOCK_SIZE_SHIFT_L0) & 0x1FF; 
    uint64_t l1_index = ((uint64_t) va >> BLOCK_SIZE_SHIFT_L1) & 0x1FF;
    uint64_t l2_index = ((uint64_t) va >> BLOCK_SIZE_SHIFT_L2) & 0x1FF;
    uint64_t l3_index = ((uint64_t) va >> BLOCK_SIZE_SHIFT_L3) & 0x1FF;

    //Check if index l0 is invalide. If yes initalize page table
    if(!(root_pagetable[l0_index] & 1)){
        /* if(verbose){
            kernel_logger_log("There is no an initalized page table at index l0: %i", l0_index);
        } */
        root_pagetable[l0_index] = (int64_t) map_page_for_pagetable() | PAGETABLEATTRIBUTES;
    }
    uintptr_t l1_phy_addr = root_pagetable[l0_index] & 0x0000FFFFFFFFF000ULL;
    uint64_t* l1 = phys_to_virt(l1_phy_addr);

    if(!(l1[l1_index] & 1)){
        if(verbose){
            kernel_logger_log_no_terminal(0, "There is no initialized page table at index l1:%u", l1_index);
        }
        uint16_t t = 1;
        l1[l1_index] = (int64_t) map_page_for_pagetable() | PAGETABLEATTRIBUTES;
        if(verbose){
            kernel_logger_log_no_terminal("Map page table l2.");
        }
    }
    uintptr_t l2_phy_addr = l1[l1_index] & 0x0000FFFFFFFFF000ULL;
    uint64_t* l2 = phys_to_virt(l2_phy_addr);

    if(!(l2[l2_index] & 1)){
        if(verbose){
            kernel_logger_log_no_terminal("There is no initialized page table at index l2: %u", l2_index);
        }
        l2[l2_index] = (int64_t) map_page_for_pagetable() | PAGETABLEATTRIBUTES;
        if(verbose){
            kernel_logger_log_no_terminal("Map page table l3.");
        }
    }
    uintptr_t l3_phy_addr = l2[l2_index] & 0x0000FFFFFFFFF000ULL;
    uint64_t* l3 = phys_to_virt(l3_phy_addr);

    if(!(l3[l3_index] & 1)){
        if(verbose){
            kernel_logger_log_no_terminal("There is no initialized page at index l3: %u", l3_index);
        }
        if(attr_idx==0){
            uintptr_t block = PAGEATTRIBUTESFORNORMALMEMORY | pa;
            l3[l3_index] = (uint64_t)block;
            if(verbose){
                kernel_logger_log_no_terminal("Created page at idx0: %u, idx1: %u, idx2: %u, idx3: %u", l0_index, l1_index, l2_index, l3_index);
            }
        }
        else if(attr_idx == 1){
            uint64_t block = PAGEATTRIBUTESFORDEVICEMEMORY | pa;
            l3[l3_index] = (uint64_t)block;
            if(verbose){
                kernel_logger_log_no_terminal("Created page at idx0: %u, idx1: %u, idx2: %u, idx3: %u", l0_index, l1_index, l2_index, l3_index);
            }
        }
        else{
            kernel_logger_log_no_terminal("The memory attribute index %u does not exist.", attr_idx);
        }
        return;
    }
    kernel_logger_log_no_terminal("Error: Page already initialized");
}


/**Map a specified range of memory 
 * TODO Move this to the page allocator
*/
void map_addr(uintptr_t address, uint64_t size){
    if(size%PAGESIZE != 0){
        kernel_logger_log_no_terminal("Size is not page aligned.");
        return;
    }
    uint64_t leftover = size;
    uintptr_t adr = address;
    while(leftover > 0){
        mmu_map_4kb(l0_table, adr, adr, 0);
        leftover -= BLOCK_SIZE_L3;
        adr += BLOCK_SIZE_L3;
    }
    /* uint64_t leftover = size;
    if(size>=BLOCK_SIZE_L1){
        mmu_map_1gb(l0_table, address, address, 0);
        leftover = size - BLOCK_SIZE_L1;
        if(leftover > 0){
            map_addr(address+BLOCK_SIZE_L1, leftover);
        }
    }
    else if(size>=BLOCK_SIZE_L2){
        mmu_map_2mb(l0_table, address, address, 0);
        uint64_t leftover = size - BLOCK_SIZE_L2;
        if(leftover > 0){
            map_addr(address+BLOCK_SIZE_L2, leftover);
        }
    }
    else if(size>=BLOCK_SIZE_L3){ */
        /* mmu_map_4kb(l0_table, address, address, 0);
        uint64_t leftover = size - BLOCK_SIZE_L3;
        if(leftover > 0){
            map_addr(address+BLOCK_SIZE_L3, leftover);
        } */
    //}
    if(verbose){
        if(leftover==0){
            kernel_logger_log_no_terminal("Finished mapping.");
        }
    }
}

/**
 * 
 */
static uint64_t* map_page_for_pagetable(){
    //TODO: Currently I just allocate 4 pages eventhough the higher level page tables are not needed. This has to be optimized in the future. A possible solution would be to just release the unused pages again
    uintptr_t continueous_block = (uintptr_t)palloc_continuous_block(4);
    struct map_new_page_tables new_page_tables = {(uint64_t*)continueous_block, (uint64_t*)(continueous_block+3*PAGESIZE), (uint64_t*)(continueous_block+2*PAGESIZE), (uint64_t*)(continueous_block+PAGESIZE)};
    uint64_t l0_index = ((uint64_t) new_page_tables.new_pagetable >> BLOCK_SIZE_SHIFT_L0) & 0x1FF; 
    uint64_t l1_index = ((uint64_t) new_page_tables.new_pagetable >> BLOCK_SIZE_SHIFT_L1) & 0x1FF;
    uint64_t l2_index = ((uint64_t) new_page_tables.new_pagetable >> BLOCK_SIZE_SHIFT_L2) & 0x1FF;
    uint64_t l3_index = ((uint64_t) new_page_tables.new_pagetable >> BLOCK_SIZE_SHIFT_L3) & 0x1FF;
    if(!(l0_table[l0_index] & 1)){
        map_page_for_table_h(l0_table, new_page_tables, 1);
        uintptr_t l1_table = virt_to_phys((uint64_t*) new_page_tables.new_l1_pagetable);
        l0_table[l0_index] = (uint64_t)l1_table | PAGETABLEATTRIBUTES;
        zero_out_page(tmp_l1_table);
        zero_out_page(tmp_l2_table);
        zero_out_page(tmp_l3_table);
        return new_page_tables.new_pagetable;
    }
    uintptr_t l1_phy_addr = l0_table[l0_index] & 0x0000FFFFFFFFF000ULL;
    uint64_t* l1_table = phys_to_virt(l1_phy_addr);
    if(!(l1_table[l1_index] & 1)){
        map_page_for_table_h(l1_table, new_page_tables, 2);
        uintptr_t l2_table = virt_to_phys((uint64_t*) new_page_tables.new_l2_pagetable);
        l1_table[l1_index] = (uint64_t)l2_table | PAGETABLEATTRIBUTES;
        zero_out_page(tmp_l1_table);
        zero_out_page(tmp_l2_table);
        return new_page_tables.new_pagetable;
    }
    uintptr_t l2_phy_addr = l1_table[l1_index] & 0x0000FFFFFFFFF000ULL;
    uint64_t* l2_table = phys_to_virt(l2_phy_addr);
    if(!(l2_table[l2_index] & 1)){
        map_page_for_table_h(l2_table, new_page_tables, 3);
        uintptr_t l3_table = virt_to_phys((uint64_t*) new_page_tables.new_l3_pagetable);
        l2_table[l2_index] = (uint64_t)l3_table | PAGETABLEATTRIBUTES;
        zero_out_page(tmp_l1_table);
        return new_page_tables.new_pagetable;
    }
    uintptr_t l3_phy_addr = l2_table[l2_index] & 0x0000FFFFFFFFF000ULL;
    uint64_t* l3_table = phys_to_virt(l3_phy_addr);
    if(!(l3_table[l3_index] & 1)){
        l3_table[l3_index] = PAGEATTRIBUTESFORNORMALMEMORY | virt_to_phys(new_page_tables.new_pagetable);
        return new_page_tables.new_pagetable;
    }
    
}


/**
 * Internal helper for mapping a page-table page into the translation hierarchy.
 *
 * Starting from `parent_pagetable`, this function creates or updates the
 * required table entries for `page_to_map` at the given translation-table
 * `level` and links the result into `target_table`.
 *
 * @param parent_pagetable Parent page table whose entry is updated.
 * @param target_table Page table that receives the final table link. This page table will replace the temporary one.
 * @param page_tables A struct that contains the new pagetables.
 * @param level Translation-table level to process (`1` = L1, `2` = L2, `3` = L3).
 */
static void map_page_for_table_h(uint64_t* parent_pagetable, struct map_new_page_tables page_tables, uint8_t level){
    uint64_t p3_l0_index = ((uint64_t) page_tables.new_l3_pagetable >> BLOCK_SIZE_SHIFT_L0) & 0x1FF; 
    uint64_t p3_l1_index = ((uint64_t) page_tables.new_l3_pagetable >> BLOCK_SIZE_SHIFT_L1) & 0x1FF;
    uint64_t p3_l2_index = ((uint64_t) page_tables.new_l3_pagetable >> BLOCK_SIZE_SHIFT_L2) & 0x1FF;
    uint64_t p3_l3_index = ((uint64_t) page_tables.new_l3_pagetable >> BLOCK_SIZE_SHIFT_L3) & 0x1FF;
    // If indices for l0,l1,l2 of p1,p2,p3 are not the same execute special procedure to handle error

    if(level == 1){
        if(!(parent_pagetable[p3_l0_index] & 1)){
            uint64_t p1_l3_index = ((uint64_t) page_tables.new_l1_pagetable >> BLOCK_SIZE_SHIFT_L3) & 0x1FF;
            uintptr_t l1_table_tmp = virt_to_phys((uint64_t*)tmp_l1_table);
            parent_pagetable[p3_l0_index] = (uint64_t)l1_table_tmp | PAGETABLEATTRIBUTES;

            map_page_for_table_h(tmp_l1_table, page_tables, 2);

            uintptr_t page = PAGEATTRIBUTESFORNORMALMEMORY | virt_to_phys(page_tables.new_l1_pagetable);
            page_tables.new_l3_pagetable[p1_l3_index] = (uint64_t)page;//Map l1 page table to memory over the temporary tables
            zero_out_page(page_tables.new_l1_pagetable);

            uintptr_t l2_table = virt_to_phys((uint64_t*)page_tables.new_l2_pagetable);
            page_tables.new_l1_pagetable[p3_l1_index] = (uint64_t)l2_table | PAGETABLEATTRIBUTES; //Map l2 table into l1 table
            
        }
        else{
            kernel_logger_log_no_terminal("Error: MMU: The page table at level 1, for your page table mapping is already mapped.");
        }
    }
    else if(level == 2){
        if(!(parent_pagetable[p3_l1_index] & 1)){
            uint64_t p2_l3_index = ((uint64_t) page_tables.new_l2_pagetable >> BLOCK_SIZE_SHIFT_L3) & 0x1FF;
            uintptr_t l2_table_tmp = virt_to_phys((uint64_t*)tmp_l2_table);
            parent_pagetable[p3_l1_index] = (uint64_t)l2_table_tmp | PAGETABLEATTRIBUTES;
            
            map_page_for_table_h(tmp_l2_table, page_tables, 3);

            uintptr_t page = PAGEATTRIBUTESFORNORMALMEMORY | virt_to_phys(page_tables.new_l2_pagetable);
            page_tables.new_l3_pagetable[p2_l3_index] = (uint64_t)page; //Map l2 page table to memory over the temporary tables
            zero_out_page(page_tables.new_l2_pagetable); //Since it is now mapped we can clean it
            
            uintptr_t l3_table = virt_to_phys((uint64_t*)page_tables.new_l3_pagetable);
            page_tables.new_l2_pagetable[p3_l2_index] = (uint64_t)l3_table | PAGETABLEATTRIBUTES; //Map l3 table into l2 table
        }
        else{
            kernel_logger_log_no_terminal("Error: MMU: The page table at level 2, for your page table mapping is already mapped.");
        }
    }
    if(level == 3){
        if(!(parent_pagetable[p3_l2_index] & 1)){
            uint64_t p3_new_pagetable_index = ((uint64_t) page_tables.new_pagetable >> BLOCK_SIZE_SHIFT_L3) & 0x1FF;
            uintptr_t l3_table_tmp = virt_to_phys((uint64_t*)tmp_l3_table);
            parent_pagetable[p3_l2_index] = (uint64_t)l3_table_tmp | PAGETABLEATTRIBUTES;

            uintptr_t page = PAGEATTRIBUTESFORNORMALMEMORY | virt_to_phys(page_tables.new_l3_pagetable);
            tmp_l3_table[p3_l3_index] = (uint64_t)page; // Map page for l3 pagetable
            zero_out_page(page_tables.new_l3_pagetable);
            page_tables.new_l3_pagetable[p3_l3_index] = (uint64_t)page; // Map page for l3 page table
            page_tables.new_l3_pagetable[p3_new_pagetable_index] = PAGEATTRIBUTESFORNORMALMEMORY | virt_to_phys(page_tables.new_pagetable);
            
        }
        else{
            kernel_logger_log_no_terminal("Error: MMU: The page table at level 3, for your page table mapping is already mapped.");
        }
    }
}

void unmap_addr(){


}

/**
 * Maps a device-memory range with identity mappings.
 *
 * Uses 1 GB, 2 MB, or 4 KB mappings depending on the remaining size.
 * The size must be aligned to `PAGESIZE`.
 *
 * @param address Start address of the device-memory range.
 * @param size Size of the range in bytes.
 */
void register_device_memory(uintptr_t address, uint32_t size){
    uint64_t leftover = size;
    uintptr_t adr = address;
    while(leftover > 0){
        mmu_map_4kb(l0_table, adr, adr, 1);
        leftover -= BLOCK_SIZE_L3;
        adr += BLOCK_SIZE_L3;
    }
    /* if(size%PAGESIZE != 0){
        kernel_logger_log("Size is not page aligned.");
        return;
    }
    uint64_t leftover = size;
    if(size>=BLOCK_SIZE_L1){
        mmu_map_1gb(l0_table, address, address, 1);
        leftover = size - BLOCK_SIZE_L1;
        if(leftover > 0){
            register_device_memory(address+BLOCK_SIZE_L1, leftover);
        }
    }
    else if(size>=BLOCK_SIZE_L2){
        mmu_map_2mb(l0_table, address, address, 1);
        uint64_t leftover = size - BLOCK_SIZE_L2;
        if(leftover > 0){
            register_device_memory(address+BLOCK_SIZE_L2, leftover);
        }
    }
    else if(size>=BLOCK_SIZE_L3){
        mmu_map_4kb(l0_table, address, address, 1);
        uint64_t leftover = size - BLOCK_SIZE_L3;
        if(leftover > 0){
            register_device_memory(address+BLOCK_SIZE_L3, leftover);
        }
    } 
        mmu_map_4kb(l0_table, address, address, 1);
        uint64_t leftover = size - BLOCK_SIZE_L3;
        if(leftover > 0){
            register_device_memory(address+BLOCK_SIZE_L3, leftover);
        }
    */
    if(verbose){
        if(leftover==0){
            kernel_logger_log_no_terminal("Finished device memory mapping.");
        }
    }
}

/**Map physical addresses to virtual ones. */
static uint64_t* phys_to_virt(uintptr_t phy_addr){
    return (uint64_t*) phy_addr;
}

/**Map virtual addresses to physical ones. */
static uintptr_t virt_to_phys(uint64_t* phy_addr){
    return (uintptr_t) phy_addr;
}


void mmu_verbose(int enable){
    verbose = enable;
}

static void zero_out_page(uint64_t* page){
    for(int i = 0; i<PAGESIZE / sizeof(*page); ++i){
        page[i] = 0;
    }
}




/*
TODO
Clear page table entries
Recursive page table mapping
*/