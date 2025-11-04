#include "uart.h"
#include "pci_driver.h"
#include "virtio_gpu_driver.hpp"
#include "allocator.h"


extern uintptr_t stack_top;
extern uintptr_t stack_bottom;
extern uintptr_t heap_start;
extern uintptr_t heap_end;

extern "C" void kernel_main(void) {
    init();
    struct bus_device_function bdf = search_pci_device(0x1050, 0x1af4, 1);
    print_pci_config_space_header(bdf);
    put_uart("###############################", 1);
    virtio_gpu_driver gpu_driver;
    put_uart("z", 1);
}