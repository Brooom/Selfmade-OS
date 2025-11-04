#include "pci_driver.h"
#include "uart.h"


volatile uint8_t* ecam_ptr(const struct bus_device_function bdf, uint16_t off) {
    // Every number indicates a byte
    return (volatile uint8_t*)(
        ECAM_BASE + 
        ((uintptr_t)bdf.bus_nr << 20) + 
        ((uintptr_t)bdf.device_nr << 15) + 
        ((uintptr_t)bdf.function_nr << 12) + 
        off);
}

struct bus_device_function search_pci_device(uint16_t device_id, uint16_t vendor_id, bool verbose){
    for(int i=0; i<NR_OF_BUSES; i++){
        for(int j=0; j<NR_OF_DEVICES; j++){
            for(int k=0; k<NR_OF_FUNCTIONS; k++){
                struct bus_device_function bdf = (struct bus_device_function){i, j, k};
                volatile uint32_t* pointer = (volatile uint32_t*)ecam_ptr(bdf, 0);
                if(verbose == 1){
                    char buf[12];
                    put_uart("bus: ", 0);
                    int_to_str(i, buf);
                    put_uart(buf, 0);
                    put_uart(", dev: ", 0);
                    int_to_str(j, buf);
                    put_uart(buf, 0);
                    put_uart(", fn: ", 0);
                    int_to_str(k, buf);
                    put_uart(buf, 0);
                    put_uart(", Value: ", 0);
                    u32_to_hex(*pointer, buf);
                    put_uart(buf, 1);
                }
                if(((uint32_t)device_id << 16 | vendor_id) == *pointer){
                    if(verbose == 1) put_uart("Found device.", 1);
                    return bdf;
                }
            }
        }
    }
}

void print_pci_config_space_header(struct bus_device_function bdf){
    for(int i = 0; i<16; ++i){
        volatile uint32_t* pointer = (volatile uint32_t*)ecam_ptr(bdf, i*4);
        uart_send_hex_value(pointer, sizeof(*pointer));
    }
}

static uint8_t get_pci_capability_offset(struct bus_device_function bdf){
    volatile uint8_t* cap_offset = ecam_ptr(bdf, 0x34);
    return *cap_offset & 0xFC;
}

struct virtio_pci_cap* get_pci_capability(struct bus_device_function bdf, uint8_t cfg_type, bool verbose){
    int i = 0;
    uint8_t cap_offset = get_pci_capability_offset(bdf);
    volatile uint8_t *p = ecam_ptr(bdf, cap_offset);
    struct virtio_pci_cap *cap = (struct virtio_pci_cap*)p;
    while((cap->cfg_type) != cfg_type){
        if(verbose == 1){
            put_uart("\n", 0);
            put_uart("Capability     ", 0);
            uart_send_dec_value(i);
            put_uart("Capability id: ", 0);
            uart_send_hex_value(&cap->cap_vndr, sizeof(cap->cap_vndr));
            put_uart("next:          ", 0);
            uart_send_hex_value(&cap->cap_next, sizeof(cap->cap_next));
            put_uart("bar:           ", 0);
            uart_send_hex_value(&cap->bar, sizeof(cap->bar));
            put_uart("config type:   ", 0);
            uart_send_hex_value(&cap->cfg_type, sizeof(cap->cfg_type));
            put_uart("Offset:        ", 0);
            uart_send_hex_value(&cap->offset, sizeof(cap->offset));
            put_uart("\n", 0);
        }
        p = ecam_ptr(bdf, cap->cap_next & 0xFC);
        cap = (struct virtio_pci_cap*)p;
        if((cap->cap_next & 0xFC) == 0x0){
            put_uart("Could not find this capability type.", 1);
            break;
        }
    }
    if(verbose == 1){
        put_uart("\n", 0);
        put_uart("Capability     ", 0);
        uart_send_dec_value(i);
        put_uart("Capability id: ", 0);
        uart_send_hex_value(&cap->cap_vndr, sizeof(cap->cap_vndr));
        put_uart("next:          ", 0);
        uart_send_hex_value(&cap->cap_next, sizeof(cap->cap_next));
        put_uart("bar:           ", 0);
        uart_send_hex_value(&cap->bar, sizeof(cap->bar));
        put_uart("config type:   ", 0);
        uart_send_hex_value(&cap->cfg_type, sizeof(cap->cfg_type));
        put_uart("Offset:        ", 0);
        uart_send_hex_value(&cap->offset, sizeof(cap->offset));
        put_uart("\n", 0);
    }
    return cap;
}

static uint64_t BAR_size(struct bus_device_function bdf, uint8_t bar_index, bool verbose){
    volatile uint32_t *BAR = (volatile uint32_t*)ecam_ptr(bdf, 0x10);
    uint64_t size;
    volatile uint16_t *coomand_register = (volatile uint16_t*)ecam_ptr(bdf, 0x04);
    uint16_t coomand_register_before = *coomand_register;
    *coomand_register &= 0xFFFC;

    //Memory Space BAR with 32 bit
    if((BAR[bar_index] & 0x0000007) == 0){
        uint32_t BAR_value_before = BAR[bar_index];
        BAR[bar_index] = 0xFFFFFFFF;
        size = (uint64_t) ~(BAR[bar_index] & 0xFFFFFFF0)+1;
        BAR[bar_index] = BAR_value_before;
    }

    //Memory Space BAR with 64 bit
    else if((BAR[bar_index] & 0x0000007)==0x4){ 
        uint32_t BAR_value_before1 = BAR[bar_index];
        uint32_t BAR_value_before2 = BAR[bar_index+1];
        BAR[bar_index] = 0xFFFFFFFF;
        BAR[bar_index+1] = 0xFFFFFFFF;
        size = ~(((uint64_t)BAR[bar_index+1])<<32 | (BAR[bar_index] & 0xFFFFFFF0))+1;
        BAR[bar_index] = BAR_value_before1;
        BAR[bar_index+1] = BAR_value_before2;
        *coomand_register = coomand_register_before;
    }
    else{
        put_uart("Other implementation needed", 1);
        size = 0;
    }
    *coomand_register = coomand_register_before;
    return size;
}

uint64_t* alloc_bar_memory(struct bus_device_function bdf, uint8_t bar_index, bool verbose){
    //memory types:
    //1: IO    2: non prefetchable    3: prefetchable
    volatile uint32_t *start_BAR = (volatile uint32_t*)ecam_ptr(bdf, 0x10);
    volatile uint32_t *BAR = start_BAR + bar_index;
    if(verbose == 1){
        put_uart("gggggggggggg ", 0);
        uart_send_hex_value(BAR, sizeof(*BAR));
    }
    uint8_t memory_type;
    if((*BAR & 1) == 1){
        // IO
        memory_type = 1;
    }
    else if((*BAR & 9) == 8){
        // Prefetchable
        memory_type = 3;    
    }
    else if((*BAR & 9) == 0){
        // Non-prefetchable
        memory_type = 2; 
    }
    else{
        put_uart("Wrong memory type:", 1);
    }

    // Address length: 0 32-bit 1 reserved 2 64-bit
    uint8_t address_length = (*BAR & 6) >> 1;
    if(verbose == 1){
        put_uart("address length: ", 0);
        uart_send_dec_value(address_length);
    }

    if(verbose == 1){
        put_uart("Memory type: ", 0);
        uart_send_dec_value(memory_type);
    }
    uint64_t bar_size = BAR_size(bdf, bar_index, 1);
    uint64_t offset = bar_size;
    if(memory_type == 1){
        if(address_length == 0){
            if(CPU_PCI_IO_BASE_ADDRESS+offset+bar_size < CPU_PCI_IO_BASE_ADDRESS+CPU_PCI_IO_SIZE){
                BAR[bar_index] = (uint32_t) BUS_PCI_IO_BASE_ADDRESS+offset;
                return (uint_fast64_t*) CPU_PCI_IO_BASE_ADDRESS+offset;
            }
        }
        else{
            put_uart("Address length 64-bit for memory type 1 not implemented so far", 1);
        }
    }
    else if(memory_type == 2){
        if(address_length == 0){
            if(CPU_PCI_MEMORY_NON_PREFETCHABLE_BASE+offset+bar_size < CPU_PCI_MEMORY_NON_PREFETCHABLE_BASE+CPU_PCI_MEMORY_NON_PREFETCHABLE_SIZE){
                BAR[bar_index] = (uint32_t) BUS_PCI_MEMORY_NON_PREFETCHABLE_BASE+offset;
                return (uint_fast64_t*) CPU_PCI_MEMORY_NON_PREFETCHABLE_BASE+offset;
            }
        }
        else{
            put_uart("Address length 64-bit for memory type 2 not implemented so far", 1);
        }
    }
    else if(memory_type == 3){
        if(address_length == 2){
            if(CPU_PCI_MEMORY_PREFETCHABLE_BASE+offset+bar_size < CPU_PCI_MEMORY_PREFETCHABLE_BASE+CPU_PCI_MEMORY_PREFETCHABLE_SIZE){

                uint64_t v = (uint64_t) CPU_PCI_MEMORY_PREFETCHABLE_BASE+offset;
                BAR[0] = (uint32_t) v;
                BAR[1] = (uint32_t) (v>>32);
                return (uint_fast64_t*) v;
            }
        }
        else{
            put_uart("Address length 64-bit for memory type 1 not implemented so far", 1);
        }
    }
    else{
        put_uart("Wrong memory type.", 1);
        return NULL;
    }
}