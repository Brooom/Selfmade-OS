#ifndef PCI_DRIVER_H
#define PCI_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

static const int NR_OF_BUSES = 256;
static const int NR_OF_DEVICES = 32;
static const int NR_OF_FUNCTIONS = 8;
static const unsigned long ECAM_BASE = 0x0000004010000000ULL;

struct bus_device_function{
    int bus_nr;
    int device_nr;
    int function_nr;
};

struct virtio_pci_cap{ 
    uint8_t cap_vndr; /* Generic PCI field: PCI_CAP_ID_VNDR */ 
    uint8_t cap_next; /* Generic PCI field: next ptr. */ 
    uint8_t cap_len; /* Generic PCI field: capability length */ 
    uint8_t cfg_type; /* Identifies the structure. */ 
    uint8_t bar; /* Where to find it. */
    uint8_t id; /* Multiple capabilities of the same type */ 
    uint8_t padding[2]; /* Pad to full dword. */ 
    uint32_t offset; /* Offset within bar. */ 
    uint32_t length; /* Length of the structure, in bytes. */ 
};

/** Gives the pointer to an Configuration Space Header element*/
volatile uint8_t* ecam_ptr(const struct bus_device_function bdf, uint16_t off);

/** Returns the bus id, device id and function id for the device you are looking for. */
struct bus_device_function search_pci_device(uint16_t device_id, uint16_t vendor_id, bool verbose);

/** Print whole config space header */
void print_pci_config_space_header(struct bus_device_function bdf);


/* Common configuration */
#define VIRTIO_PCI_CAP_COMMON_CFG        1
/* Notifications */
#define VIRTIO_PCI_CAP_NOTIFY_CFG        2
/* ISR Status */
#define VIRTIO_PCI_CAP_ISR_CFG           3
/* Device specific configuration */
#define VIRTIO_PCI_CAP_DEVICE_CFG        4
/* PCI configuration access */
#define VIRTIO_PCI_CAP_PCI_CFG           5
/* Shared memory region */
#define VIRTIO_PCI_CAP_SHARED_MEMORY_CFG 8
/* Vendor-specific data */
#define VIRTIO_PCI_CAP_VENDOR_CFG        9

/** Get pci capability of specific type defined by */
struct virtio_pci_cap* get_pci_capability(struct bus_device_function bdf, uint8_t cfg_type, bool verbose);

#define CPU_PCI_IO_BASE_ADDRESS                  UINT64_C(0x000000003EFF0000)
#define CPU_PCI_IO_SIZE                          UINT64_C(0x0000000000010000)
#define BUS_PCI_IO_BASE_ADDRESS                  UINT32_C(0x00000000)

#define CPU_PCI_MEMORY_NON_PREFETCHABLE_BASE     UINT64_C(0x0000000010000000)
#define CPU_PCI_MEMORY_NON_PREFETCHABLE_SIZE     UINT64_C(0x000000002EFF0000)
#define BUS_PCI_MEMORY_NON_PREFETCHABLE_BASE     UINT32_C(0x10000000)

#define CPU_PCI_MEMORY_PREFETCHABLE_BASE         UINT64_C(0x0000008000000000)
#define CPU_PCI_MEMORY_PREFETCHABLE_SIZE         UINT64_C(0x0000008000000000)
#define BUS_PCI_MEMORY_PREFETCHABLE_BASE         UINT64_C(0x0000008000000000)
uint64_t* alloc_bar_memory(struct bus_device_function bdf, uint8_t bar_index, bool verbose);

/** Get pci capability offset */
static uint8_t get_pci_capability_offset(struct bus_device_function bdf);
/** Get size of the memory one has to allocate for the base address register (bar) */
static uint64_t BAR_size(struct bus_device_function bdf, uint8_t bar_index, bool verbose);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif