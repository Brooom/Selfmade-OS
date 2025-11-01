#include "uart.h"
#include "virtio_pci.h"


extern uintptr_t stack_top;
extern uintptr_t stack_bottom;
extern uintptr_t heap_start;
extern uintptr_t heap_end;

void kernel_main(void) {
    put_uart("stack bottom: ", 0);
    uint64_t temp = (uint64_t) &stack_bottom;
    send_value_hex(&temp, sizeof(temp));

    put_uart("stack top:    ", 0);
    temp = (uint64_t) &stack_top;
    send_value_hex(&temp, sizeof(temp));

    put_uart("heap start:   ", 0);
    temp = (uint64_t) &heap_start;
    send_value_hex(&temp, sizeof(temp));

    put_uart("heap end:     ", 0);
    temp = (uint64_t) &heap_end;
    send_value_hex(&temp, sizeof(temp));

    //put_uart("Hallo world!\n", 1);
    //print_everything();
    print_config();
    char s[8];
    uint8_t cap_offset = get_capability_offset();
    put_uart("Cap offset", 1);
    send_value_hex(&cap_offset, sizeof(cap_offset));
    volatile uint8_t *p = ecam_ptr(0,2,0, cap_offset);
    capability_list(p, 0);
    int size = BAR_size(4);
    put_uart("Size of BAR:", 0);
    send_value_dec(size);
    uint64_t *pointer =  allocate(4, 3);
    // put_uart("\n", 1);
    // print_config();
    // put_uart("\n", 1);
    init_pci(pointer);
    put_uart("z", 1);

}