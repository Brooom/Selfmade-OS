ENTRY(_start)

SECTIONS {
    . = 0x40000000; /* Defines at which virtual address the image is stored. */
    .boot : { *(.boot) } /* Places the boot script at . */
    .text : { *(.text) } /* Places all the remaining text sections after the boot section. So in my case the .c files */
    .data : { *(.data) } /* Same as with the text section with the data section */
    . = ALIGN(64);
    vector_table = .;
    .vectors : { KEEP(*(.vectors)) }
    .bss  : { *(.bss) }
    . = ALIGN(16); /* Rounds the location counter up */
    kernel_stack_bottom = .;
    . = . + 0x10000; /* Increases the location counter by 4096 bytes. Such that the stack has that much space since the stack grows downwards. */
    /* Note: Every .blablabla increases the location counter */
    kernel_stack_top = .;
    . = . + 1;
    . = ALIGN(16);
    kernel_heap_start = .;
    . = . + 0x10000;
    kernel_heap_end = 0x50000000;
    . = kernel_heap_end;
}
