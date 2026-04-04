# MMU
## Concept
During boot up not all the normal memory is mapped. The mapping is only done for memory that is allocated. This is to reduce the number of page tables and save memory. The page table mapping is also on demand.

## Page table mapping
During boot, three temporary pages are mapped and later used as temporary page tables. When a new page table is needed, the function `map_page_for_table` allocates four pages. One page is used for the new page table itself, while the other three are used for the required higher-level page tables. The temporary pages serve as intermediate page tables while these new mappings are being created. 

## Allocator
The 'kernel_allocator' handles the whole allocation process. Therefore, he calls the mmu and the 'page_allocator'. The mmu maps the memory that is going to be allocated. The 'page_allocator' allocates the memory addresses. 

## Optimization and TODO's
The 'page_allocator' is just a simple bump allocator. That should be improved to a buddy allocator.

The 'map_page_for_pagetable' function currently allocates four pages even though that only one of them is used. This is the case, when for example, there still exist free, mapped l3 pages. In this case the allocated pages for the l1, l2, and l3 tables are not needed. These pages could be freed afterward.

