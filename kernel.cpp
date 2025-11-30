#include "kernel_logs/kernel_logger.hpp"
#include "pci_driver.h"
#include "virtio_gpu_driver/virtio_gpu_driver.hpp"
#include "allocator/allocator.h"
#include "terminal.hpp"
#include "uart.h"

extern "C" void kernel_main(void) {
    kernel_logger::log("Start kernel");
    init_mmu();
    uint8_t *p = (uint8_t*) kalloc(10, 1);
}
