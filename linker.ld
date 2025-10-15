ENTRY(_start)

SECTIONS {
    . = 0x10000; /* Defines at which virtual address the image is stored. */
    .boot : { boot.o(.text) } /* Places the boot script at . */
    .text : { *(.text) } /* Places all the remaining text sections after the boot section. So in my case the .c files */
    .data : { *(.data) } /* Same as with the text section with the data section */
    .bss  : { *(.bss) }
    . = ALIGN(8); /* Rounds the location counter up */
    . = . + 0x10000; /* Increases the location counter by 4096 bytes. Such that the stack has that much space since the stack grows downwards. */
    /* Note: Every .blablabla increases the location counter */
    stack_top = .;
}
