#ifndef KERNEL_LOGGER_C_API_H
#define KERNEL_LOGGER_C_API_H

// kernel_logger_c.h
#ifdef __cplusplus
extern "C" {
#endif

void kernel_logger_log(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif