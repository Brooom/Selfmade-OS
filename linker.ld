ENTRY(_start)

SECTIONS {
    init_page_l0 = 0x40000000;
    init_page_l1 = 0x40001000;
    init_page_l2 = 0x40002000;
    init_page_l3 = 0x40003000;
    tmp_page_1 = 0x40004000;
    tmp_page_2 = 0x40005000;
    tmp_page_3 = 0x40006000;
    . = 0x40007000; /*7 pages for the mmu initalization*/
    kernel_start = ALIGN(16); /*16 byte aligned*/
    .boot : { *(.boot) } /* Places the boot script at . */
    .text : { *(.text) } /* Places all the remaining text sections after the boot section. So in my case the .c files */
    .data : { *(.data) } /* Same as with the text section with the data section */
    . = ALIGN(64);
    vector_table = .;
    .vectors : { KEEP(*(.vectors)) }
    .bss  : { *(.bss) }
    . = ALIGN(16); /* Rounds the location counter up */
    kernel_stack_bottom = ALIGN(0x1000);
    . = kernel_stack_bottom + 0x100000; /* Increases the location counter by 4096 bytes. Such that the stack has that much space since the stack grows downwards. */
    /* Note: Every .blablabla increases the location counter */
    kernel_stack_top = ALIGN(0x1000);
    kernel_end = kernel_stack_top;
    kernel_heap_start = kernel_end;
    kernel_heap_end = 0x50000000;
    . = kernel_heap_end;  
}
