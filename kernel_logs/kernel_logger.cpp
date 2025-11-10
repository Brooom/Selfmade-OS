#include "kernel_logger.hpp"
#include "kernel_logger_c_api.h"

terminal *log_terminal = nullptr;


void init_terminal(terminal *terminal)
{
    log_terminal = terminal;
}

static void vlog(const char *fmt, va_list args) {
    char output[MAX_TEXT_LENGTH];
    format_string(fmt, output, MAX_TEXT_LENGTH, args);
    if (log_terminal) log_terminal->printf("%s", output);
    uart_send(output, MAX_TEXT_LENGTH);
}

void log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vlog(fmt, args);
    va_end(args);
}

extern "C" void kernel_logger_log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vlog(fmt, args);
    va_end(args);
}
