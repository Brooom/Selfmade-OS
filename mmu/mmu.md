# MMU
## Concept
During boot up not all the normal memory is mapped. The mapping is only done for memory that is allocated. This is to reduce the number of page tables and save memory. The page table mapping is also on demand.

## Page table mapping
During boot, three pages are mapped for use as temporary page tables. These support the mapping algorithm implemented by map_page_for_pagetable, which creates a mapped page-table page when no buffered page is available.
The algorithm allocates four contiguous pages: one for the requested page table and up to three additional tables needed to establish its mapping. All four pages must fit within a single 2 MiB region so their mappings reside in the same L3 table. Since the number of required tables is not known before allocation, the complete four-page block is reserved.
The temporary tables provide access to the allocated pages while the new mapping hierarchy is constructed. Any unused pages are mapped, cleared, and stored in a buffer for later use. Subsequent requests use buffered pages first, avoiding the cost of running the mapping algorithm again.

## Allocator
The 'kernel_allocator' handles the whole allocation process. Therefore, he calls the mmu and the 'page_allocator'. The mmu maps the memory that is going to be allocated. The 'page_allocator' allocates the memory addresses. 

## Optimization and TODO's
The 'page_allocator' is just a simple bump allocator. That should be improved to a buddy allocator.

