#ifndef KERNEL_LOGGER_C_API_H
#define KERNEL_LOGGER_C_API_H

#ifdef __cplusplus
extern "C" {
#endif

void kernel_logger_log(const char *fmt, ...);
void kernel_logger_log_no_terminal(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif