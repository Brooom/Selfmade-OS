.section .vectors, "ax"
vector_table:
    b sync_exception_current_EL_using_SPEL0  
    .balign 0x80
    b irq_handler_current_EL_using_SPEL0   
    .balign 0x80    
    b fiq_handler_current_EL_using_SPEL0
    .balign 0x80
    b error_handler_current_EL_using_SPEL0
    .balign 0x80
    b sync_exception_current_EL_using_SPEL1    
    .balign 0x80  
    b irq_handler_current_EL_using_SPEL1       
    .balign 0x80   
    b fiq_handler_current_EL_using_SPEL1
    .balign 0x80
    b error_handler_current_EL_using_SPEL1
    .balign 0x80
    b sync_exception_lower_EL
    .balign 0x80
    b irq_handler_lower_EL    
    .balign 0x80
    b fiq_handler_lower_EL
    .balign 0x80
    b error_handler_lower_EL
    .balign 0x80
    b sync_exception_lower_EL_all_lower_EL_AARCH32
    .balign 0x80
    b irq_handler_lower_EL_all_lower_EL_AARCH32    
    .balign 0x80
    b fiq_handler_lower_EL_all_lower_EL_AARCH32
    .balign 0x80
    b error_handler_lower_EL_all_lower_EL_AARCH32

.section .boot, "ax"
.global _start

_start:
//Check on which EL the program is (currently EL1)
//mrs x0, CurrentEL
//str x0, [sp, #16]


msr daifclr, #0xf   // clear D, A, I, F bits
ldr x0, =vector_table
msr VBAR_EL1, x0

isb //Clear cpu cache

ldr x1,=kernel_stack_top
mov sp,x1
mov x29,xzr 
mov x20,xzr
bl kernel_main
b .
