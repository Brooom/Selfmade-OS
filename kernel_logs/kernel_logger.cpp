#include "kernel_logger.hpp"
#include "kernel_logger_c_api.h"


kernel_logger kernel_logger::klogger;

void kernel_logger::vlog(const char *fmt, va_list args, bool send_terminal) {
    char output[MAX_TEXT_LENGTH];
    format_string(fmt, output, MAX_TEXT_LENGTH, args);
    if (log_terminal && send_terminal) log_terminal->printf("%s", output);
    uart_send(output, MAX_TEXT_LENGTH);
<<<<<<< HEAD
    uart_send("\n",3);
=======
>>>>>>> c193a9f (Improve screen rendering)
}

void kernel_logger::init_terminal(terminal *terminal)
{
    klogger.log_terminal = terminal;
}

<<<<<<< HEAD
void kernel_logger::log(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    klogger.vlog(fmt, args, true);
=======
void kernel_logger::log(const char *fmt, bool send_terminal, ...)
{
    va_list args;
    va_start(args, send_terminal);
    klogger.vlog(fmt, args, send_terminal);
>>>>>>> c193a9f (Improve screen rendering)
    va_end(args);
}

kernel_logger &kernel_logger::get_logger()
{
    return klogger;
}

extern "C" void kernel_logger_log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    kernel_logger::get_logger().vlog(fmt, args);
    va_end(args);
}