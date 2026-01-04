#ifndef KERNEL_LOGGER_H
#define KERNEL_LOGGER_H


#include <stdbool.h>


#include "virtio_gpu_driver/virtio_gpu_driver.hpp"
#include "uart.h"
#include "terminal.hpp"
#include "std/args.h"

// When using this class make sure you dont introduce any circular dependencies.
class kernel_logger{
    public:
        void vlog(const char *fmt, va_list args, bool send_terminal = true);
        static void init_terminal(terminal *terminal);
        static void log(const char *fmt, ...); 
        static kernel_logger& get_logger();
    
    private:
        static kernel_logger klogger;
        terminal *log_terminal;
};



#endif
