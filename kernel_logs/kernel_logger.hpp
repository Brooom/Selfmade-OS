#ifndef KERNEL_LOGGER_H
#define KERNEL_LOGGER_H

#include "virtio_gpu_driver/virtio_gpu_driver.hpp"
#include "uart.h"
#include "terminal.hpp"
#include "std/args.h"

extern terminal *log_terminal;

void init_terminal(terminal *terminal);
void log(const char *string, ...);
static void vlog(const char *fmt, va_list args);

#endif
