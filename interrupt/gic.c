#include "gic.h"
#include "../kernel_logs/kernel_logger_c_api.h"
#include "../mmu/mmu.h"

#define GIC_DISTRIBUTOR_BASE_ADDR       0x8000000
#define GIC_DISTRIBUTOR_BASE_SIZE       0x10000
#define GIC_REDISTRIBUTOR_BASE_ADDR     0x80a000
#define GIC_REDISTRIBUTOR_BASE_SIZE     0xf60000

static uint32_t mmio_read32(uintptr_t addr)
{
    return *(volatile uint32_t *)addr;
}

static void mmio_write32(uintptr_t addr, uint32_t value)
{
    *(volatile uint32_t *)addr = value;
}


//Configure Distributor register
void config_GICD_CTLR(){
    register_device_memory(GIC_DISTRIBUTOR_BASE_ADDR, 0x10000);
    uint32_t value = mmio_read32((uintptr_t)GIC_DISTRIBUTOR_BASE_ADDR);
    kernel_logger_log("%x", value);
    *(volatile uint32_t *)GIC_DISTRIBUTOR_BASE_ADDR &= ~(0x52);
    while((value & 0x80000000)>>31!=0b0){//Wait until all changes to this register took effect
    }
    value = mmio_read32((uintptr_t)GIC_DISTRIBUTOR_BASE_ADDR+0x0004);
    kernel_logger_log("GICD_TYPER: %x", value);
    for(int n = 0; n<32; ++n){
        kernel_logger_log("GICD_ICENABLER%u: %x", n, value);
    }
    
}