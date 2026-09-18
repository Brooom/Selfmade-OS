#include "gic.h"
#include "../kernel_logs/kernel_logger_c_api.h"

void config_GICD_CTLR(){
    uint32_t value;

    __asm__ volatile(
            "mrs %0, GICD_CTLR"
            : "=r"(value)
        );
    
    kernel_logger_log_no_terminal("%i", value);
}