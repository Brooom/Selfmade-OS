#include <stdint.h>

#include "kernel_logs/kernel_logger_c_api.h"

void sync_exception_current_EL_using_SPEL0(){
    uint64_t esr, elr, far;

    asm volatile ("mrs %0, esr_el1" : "=r"(esr)); // describs the fault
    asm volatile ("mrs %0, elr_el1" : "=r"(elr)); // contains the return address
    asm volatile ("mrs %0, far_el1" : "=r"(far)); //
    uint8_t esr_ec = (uint8_t)(esr>>26)&0x3F; // Extract exception class

    kernel_logger_log("Synchronous exception:");
    kernel_logger_log("Exception from the current EL while using SP_EL0");
    kernel_logger_log("  ESR_EL1_EC: %x\n", esr_ec);
    kernel_logger_log("  ELR_EL1: %lx\n", elr);
    kernel_logger_log("  FAR_EL1: %lx\n", far);
    
}  
void irq_handler_current_EL_using_SPEL0(){
    kernel_logger_log("IRQ");
    kernel_logger_log("Exception from the current EL while using SP_EL0");
}       
void fiq_handler_current_EL_using_SPEL0(){
    kernel_logger_log("FIQ:");
    kernel_logger_log("Exception from the current EL while using SP_EL0");
}
void error_handler_current_EL_using_SPEL0(){
    kernel_logger_log("SError");
    kernel_logger_log("Exception from the current EL while using SP_EL0");
}
void sync_exception_current_EL_using_SPEL1(){
    uint64_t esr, elr, far;

    asm volatile ("mrs %0, esr_el1" : "=r"(esr)); // describs the fault
    asm volatile ("mrs %0, elr_el1" : "=r"(elr)); // contains the return address
    asm volatile ("mrs %0, far_el1" : "=r"(far)); //
    uint8_t esr_ec = (uint8_t)(esr>>26)&0x3F; // Extract exception class

    kernel_logger_log("Synchronous exception:");
    kernel_logger_log("Exception from the current EL while using SP_EL1");
    kernel_logger_log("  ESR_EL1_EC: %x\n", esr_ec);
    kernel_logger_log("  ELR_EL1: %lx\n", elr);
    kernel_logger_log("  FAR_EL1: %lx\n", far);

}      
void irq_handler_current_EL_using_SPEL1(){
    kernel_logger_log("IRQ");
    kernel_logger_log("Exception from the current EL while using SP_EL1");
}          
void fiq_handler_current_EL_using_SPEL1(){
    kernel_logger_log("FIQ:");
    kernel_logger_log("Exception from the current EL while using SP_EL1");
}
void error_handler_current_EL_using_SPEL1(){
    kernel_logger_log("SError");
    kernel_logger_log("Exception from the current EL while using SP_EL1");
}
void sync_exception_lower_EL(){
    kernel_logger_log("Synchronous exception:");
    kernel_logger_log("Exception from a lower EL and at least one lower EL is AArch64");
}
void irq_handler_lower_EL(){
    kernel_logger_log("IRQ");
    kernel_logger_log("Exception from a lower EL and at least one lower EL is AArch64");
}    
void fiq_handler_lower_EL(){
    kernel_logger_log("FIQ:");
    kernel_logger_log("Exception from a lower EL and at least one lower EL is AArch64");
}
void error_handler_lower_EL(){
    kernel_logger_log("SError");
    kernel_logger_log("Exception from a lower EL and at least one lower EL is AArch64");
}
void sync_exception_lower_EL_all_lower_EL_AARCH32(){
    kernel_logger_log("	Exception from a lower EL and all lower ELs are AArch32");
    kernel_logger_log("Synchronous exception:");
}
void irq_handler_lower_EL_all_lower_EL_AARCH32(){
    kernel_logger_log("IRQ");
    kernel_logger_log("	Exception from a lower EL and all lower ELs are AArch32");
}    
void fiq_handler_lower_EL_all_lower_EL_AARCH32(){
    kernel_logger_log("FIQ:");
    kernel_logger_log("	Exception from a lower EL and all lower ELs are AArch32");
}
void error_handler_lower_EL_all_lower_EL_AARCH32(){
    kernel_logger_log("SError");
    kernel_logger_log("	Exception from a lower EL and all lower ELs are AArch32");

}
