#ifndef VIRTIO_PCI_H
#define VIRTIO_PCI_H

#include <stdint.h>
#include <stddef.h>

struct virtio_pci_cap;
struct virtq_desc;
struct virtio_pci_common_cfg;
struct virtq_desc;
struct virtq_avail;
struct virtq_used_elem;
struct virtq_used;
struct virtq;
struct virtio_gpu_resp_display_info;
struct virtio_gpu_display_one;
struct virtio_gpu_rect;

void print_everything(void);
volatile uint8_t* ecam_ptr(uint8_t bus, uint8_t dev, uint8_t fn, uint16_t off);
void print_config();
void print_val(uint8_t bus, uint8_t dev, uint8_t fn, uint16_t off);
uint32_t pci_read32(uint8_t b, uint8_t d, uint8_t f, uint16_t o);

void int_to_str(int value, char *buf);
void u64_to_hex(uint64_t value, char *buf);
void u32_to_hex(uint32_t value, char *buf);
void u16_to_hex(uint16_t value, char *buf);
void u8_to_hex(uint8_t value, char *buf);

uint8_t get_capability_offset();
bool read_virtio_cap(uint8_t cap_off, struct virtio_pci_cap *out);
void send_value_hex(volatile void *p, size_t size);
void send_value_dec(int v);
void capability_list(volatile uint8_t *p, uint8_t i);
uint64_t BAR_size(uint8_t bar_index);
uint64_t* allocate(uint8_t bar_index, uint8_t memory_type);
void init_pci(uint64_t *bar4);
struct virtq construct_virtqueue(volatile struct virtio_pci_common_cfg *common_cfg);
void supply_buffer_to_device(struct virtq *virtq, volatile struct virtio_pci_common_cfg *common_cfg, uint64_t *bar4, struct virtio_gpu_resp_display_info* info0);
struct virtio_pci_notify_cap* get_notify_cap(volatile uint8_t *p);
#endif /* VIRTIO_PCI_H */
