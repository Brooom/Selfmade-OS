
.global _start


_start:
    ldr     r0, =stack_top
    mov     sp, r0
    bl      kernel_main
    b       .





@ .text
@ .global _start

@ _start:
@ ldr x1,=stack_top
@ mov sp,x1
@ mov x29,xzr 
@ mov x20,xzr
@ bl kernel_main
@ b .
