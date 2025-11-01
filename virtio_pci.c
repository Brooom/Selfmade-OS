#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "uart.h"
#include "virtio_pci.h"
#include "allocator.h"
#define NR_OF_BUSES 256
#define NR_OF_DEVICES 32
#define NR_OF_FUNCTIONS 8
#define HEADER_SIZE 1u<<12
#define ECAM_BASE 0x0000004010000000ULL //UL makes it unsigned long

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
struct virtio_pci_cap { 
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


#define VIRTQ_DESC_F_NEXT     1  /* bit 0 */
#define VIRTQ_DESC_F_WRITE    2  /* bit 1 */
#define VIRTQ_DESC_F_INDIRECT 4  /* bit 2 */


struct virtio_pci_common_cfg {
    /* About the whole device. */
    uint32_t device_feature_select;   /* read-write */
    uint32_t device_feature;          /* read-only for driver */
    uint32_t driver_feature_select;   /* read-write */
    uint32_t driver_feature;          /* read-write */
    uint16_t config_msix_vector;      /* read-write */
    uint16_t num_queues;              /* read-only for driver */
    uint8_t   device_status;           /* read-write */
    uint8_t   config_generation;       /* read-only for driver */

    /* About a specific virtqueue. */
    uint16_t queue_select;            /* read-write */
    uint16_t queue_size;              /* read-write */
    uint16_t queue_msix_vector;       /* read-write */
    uint16_t queue_enable;            /* read-write */
    uint16_t queue_notify_off;        /* read-only for driver */
    uint64_t queue_desc;              /* read-write */
    uint64_t queue_driver;            /* read-write */
    uint64_t queue_device;            /* read-write */
    uint16_t queue_notify_data;       /* read-only for driver */
    uint16_t queue_reset;             /* read-write */
};

struct virtq_desc {
    uint64_t addr;   /* Guest physical address of buffer */
    uint32_t len;    /* Length */
    uint16_t flags;  /* VIRTQ_DESC_F_* */
    uint16_t next;   /* Next descriptor in chain (if NEXT set) */
};

/* Driver -> device: ring of available buffers (descriptor indices). */
struct virtq_avail {
    uint16_t flags;
    uint16_t idx;      /* Driver increments when adding entries to ring[] */
    uint16_t ring[];   /* descriptor indices, size = queue_size */
    /* Only present if VIRTIO_F_EVENT_IDX negotiated: */
    /* le16 used_event;  // driver wants interrupt when used.idx reaches this */
};

/* Device -> driver: one completion entry. */
struct virtq_used_elem {
    uint32_t id;   /* Index of start of a descriptor chain */
    uint32_t len;  /* Total bytes written by device into that chain */
};

/* Device -> driver: ring of used (completed) buffers. */
struct virtq_used {
    uint16_t flags;
    uint16_t idx;             /* Device increments when adding entries to ring[] */
    struct virtq_used_elem ring[]; /* size = queue_size */
    /* Only present if VIRTIO_F_EVENT_IDX negotiated: */
    /* le16 avail_event;  // device wants notification when avail.idx reaches this */
};

/* A handy wrapper describing a queue (not wire format). */
struct virtq {
    unsigned int num;            /* queue size (power of two) */
    struct virtq_desc *desc;     /* num descriptors */
    struct virtq_avail *avail;   /* avail ring (with num entries) */
    struct virtq_used  *used;    /* used ring (with num entries) */
};

#define CPU_PCI_IO_BASE_ADDRESS                  UINT64_C(0x000000003EFF0000)
#define CPU_PCI_IO_SIZE                          UINT64_C(0x0000000000010000)
#define BUS_PCI_IO_BASE_ADDRESS                  UINT32_C(0x00000000)

#define CPU_PCI_MEMORY_NON_PREFETCHABLE_BASE     UINT64_C(0x0000000010000000)
#define CPU_PCI_MEMORY_NON_PREFETCHABLE_SIZE     UINT64_C(0x000000002EFF0000)
#define BUS_PCI_MEMORY_NON_PREFETCHABLE_BASE     UINT32_C(0x10000000)

#define CPU_PCI_MEMORY_PREFETCHABLE_BASE         UINT64_C(0x0000008000000000)
#define CPU_PCI_MEMORY_PREFETCHABLE_SIZE         UINT64_C(0x0000008000000000)
#define BUS_PCI_MEMORY_PREFETCHABLE_BASE         UINT64_C(0x0000008000000000)

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

enum virtio_gpu_ctrl_type {
    /* 2D commands */
    VIRTIO_GPU_CMD_GET_DISPLAY_INFO   = 0x0100,
    VIRTIO_GPU_CMD_RESOURCE_CREATE_2D,
    VIRTIO_GPU_CMD_RESOURCE_UNREF,
    VIRTIO_GPU_CMD_SET_SCANOUT,
    VIRTIO_GPU_CMD_RESOURCE_FLUSH,
    VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D,
    VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING,
    VIRTIO_GPU_CMD_RESOURCE_DETACH_BACKING,
    VIRTIO_GPU_CMD_GET_CAPSET_INFO,
    VIRTIO_GPU_CMD_GET_CAPSET,
    VIRTIO_GPU_CMD_GET_EDID,
    VIRTIO_GPU_CMD_RESOURCE_ASSIGN_UUID,
    VIRTIO_GPU_CMD_RESOURCE_CREATE_BLOB,
    VIRTIO_GPU_CMD_SET_SCANOUT_BLOB,

    /* 3D commands */
    VIRTIO_GPU_CMD_CTX_CREATE         = 0x0200,
    VIRTIO_GPU_CMD_CTX_DESTROY,
    VIRTIO_GPU_CMD_CTX_ATTACH_RESOURCE,
    VIRTIO_GPU_CMD_CTX_DETACH_RESOURCE,
    VIRTIO_GPU_CMD_RESOURCE_CREATE_3D,
    VIRTIO_GPU_CMD_TRANSFER_TO_HOST_3D,
    VIRTIO_GPU_CMD_TRANSFER_FROM_HOST_3D,
    VIRTIO_GPU_CMD_SUBMIT_3D,
    VIRTIO_GPU_CMD_RESOURCE_MAP_BLOB,
    VIRTIO_GPU_CMD_RESOURCE_UNMAP_BLOB,

    /* cursor commands */
    VIRTIO_GPU_CMD_UPDATE_CURSOR      = 0x0300,
    VIRTIO_GPU_CMD_MOVE_CURSOR,

    /* success responses */
    VIRTIO_GPU_RESP_OK_NODATA         = 0x1100,
    VIRTIO_GPU_RESP_OK_DISPLAY_INFO,
    VIRTIO_GPU_RESP_OK_CAPSET_INFO,
    VIRTIO_GPU_RESP_OK_CAPSET,
    VIRTIO_GPU_RESP_OK_EDID,
    VIRTIO_GPU_RESP_OK_RESOURCE_UUID,
    VIRTIO_GPU_RESP_OK_MAP_INFO,

    /* error responses */
    VIRTIO_GPU_RESP_ERR_UNSPEC        = 0x1200,
    VIRTIO_GPU_RESP_ERR_OUT_OF_MEMORY,
    VIRTIO_GPU_RESP_ERR_INVALID_SCANOUT_ID,
    VIRTIO_GPU_RESP_ERR_INVALID_RESOURCE_ID,
    VIRTIO_GPU_RESP_ERR_INVALID_CONTEXT_ID,
    VIRTIO_GPU_RESP_ERR_INVALID_PARAMETER,
};

struct virtio_gpu_ctrl_hdr {
    uint32_t type;      /* enum virtio_gpu_ctrl_type */
    uint32_t flags;     /* VIRTIO_GPU_FLAG_* */
    uint64_t fence_id;  /* optional fence value when FENCE flag set */
    uint32_t ctx_id;    /* 3D context identifier (for 3D commands) */
    uint8_t ring_idx;  /* control vs cursor queue (if device supports multiple) */
    uint8_t padding[3];
};

#define VIRTQ_DESC_F_NEXT 1 /* This marks a buffer as continuing. */
#define VIRTQ_DESC_F_WRITE 2 /* This marks a descriptor as device write-only (otherwise device read-only). */
#define VIRTIO_GPU_MAX_SCANOUTS 16

/* Rectangle (x, y, width, height) */
struct virtio_gpu_rect {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
};

/* Response to VIRTIO_GPU_CMD_GET_DISPLAY_INFO */
struct virtio_gpu_resp_display_info {
    struct virtio_gpu_ctrl_hdr hdr;
    struct virtio_gpu_display_one {
        struct virtio_gpu_rect r; /* scanout rectangle */
        uint32_t enabled;             /* non-zero if scanout is active */
        uint32_t flags;               /* reserved for future use */
    } pmodes[VIRTIO_GPU_MAX_SCANOUTS];
};
struct virtio_pci_notify_cap {
    struct virtio_pci_cap cap;
    uint32_t notify_off_multiplier;/* Multiplier for queue_notify_off. */ 
};


volatile uint8_t* ecam_ptr(uint8_t bus, uint8_t dev, uint8_t fn, uint16_t off) {
    // Every number indicates a byte
    return (volatile uint8_t*)(
        ECAM_BASE + 
        ((uintptr_t)bus << 20) + 
        ((uintptr_t)dev << 15) + 
        ((uintptr_t)fn  << 12) + 
        off);
}

void print_val(uint8_t bus, uint8_t dev, uint8_t fn, uint16_t off) {
    volatile uint32_t *pointer = (volatile uint32_t*)ecam_ptr(bus, dev, fn, off);
    char buf[12];
    u32_to_hex(*pointer, buf);
    put_uart(buf, 1);
}


void print_everything(){
    for(int i=0; i<NR_OF_BUSES; i++){
        for(int j=0; j<NR_OF_DEVICES; j++){
            for(int k=0; k<NR_OF_FUNCTIONS; k++){
                volatile uint32_t* pointer = (volatile uint32_t*)ecam_ptr(i,j,k,0);
                char buf[12];
                put_uart("bus: ", 0);
                int_to_str(i, buf);
                put_uart(buf, 1);
                put_uart(", dev: ", 0);
                int_to_str(j, buf);
                put_uart(buf, 1);
                put_uart(", fn: ", 0);
                int_to_str(k, buf);
                put_uart(buf, 1);
                put_uart(", Value: ", 0);
                u32_to_hex(*pointer, buf);
                put_uart(buf, 1);

            }
        }
    }
}

void print_config(){
    for(int i = 0; i<16; ++i){
        volatile uint32_t* pointer = (volatile uint32_t*)ecam_ptr(0,2,0,i*4);
        char buf[12];
        u32_to_hex(*pointer, buf);
        put_uart(buf, 1);
    }
    
}
uint32_t pci_read32(uint8_t b, uint8_t d, uint8_t f, uint16_t o) {
    return *(volatile uint32_t*)ecam_ptr(b,d,f,o);
}




void int_to_str(int value, char *buf) {
    char tmp[10];
    int len = 0;

    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    while (value > 0) {          // collect digits in reverse
        tmp[len++] = '0' + (value % 10);
        value /= 10;
    }

    for (int i = 0; i < len; ++i) // reverse into output buffer
        buf[i] = tmp[len - 1 - i];
    buf[len] = '\0';
}
void send_value_hex(volatile void *p, size_t size){
    char message[19];
    size *= 8;
    if(size==8){
        u8_to_hex(*(uint8_t*)p, message);
        put_uart(message, 1);
    }
    else if(size==16){
        u16_to_hex(*(uint16_t*)p, message);
        put_uart(message, 1);
    }
    else if(size==32){
        u32_to_hex(*(uint32_t*)p, message);
        put_uart(message, 1);
    }
    else if(size==64){
        u64_to_hex(*(uint64_t*)p, message);
        put_uart(message, 1);
    }
    else{
        put_uart("Can not convert message of size: ", 1);
        int_to_str(size, message);
        put_uart(message, 1);

    }
}

void send_value_dec(int v){
    char message[11];
    int_to_str(v, message);
    put_uart(message, 1);
    
}

void u64_to_hex(uint64_t value, char *buf) {
    const char hex[] = "0123456789abcdef";

    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 16; ++i) {
        int shift = 60- (i * 4);
        buf[2 + i] = hex[(value >> shift) & 0xF];
    }
    buf[18] = '\0';
}

void u32_to_hex(uint32_t value, char *buf) {
    const char hex[] = "0123456789abcdef";

    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 8; ++i) {
        int shift = 28 - (i * 4);
        buf[2 + i] = hex[(value >> shift) & 0xF];
    }
    buf[10] = '\0';
}

void u16_to_hex(uint16_t value, char *buf) {
    const char hex[] = "0123456789abcdef";

    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 4; ++i) {
        int shift = 12 - (i * 4);
        buf[2 + i] = hex[(value >> shift) & 0xF];
    }
    buf[6] = '\0';
}
void u8_to_hex(uint8_t value, char *buf) {
    const char hex[] = "0123456789abcdef";

    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 2; ++i) {
        int shift = 4 - (i * 4);
        buf[2 + i] = hex[(value >> shift) & 0xF];
    }
    buf[4] = '\0';
}


uint8_t get_capability_offset(){
    volatile uint8_t* cap_pointer = ecam_ptr(0,2,0,0x34);
    return *cap_pointer & 0xFC;
}

bool read_virtio_cap(uint8_t cap_off, struct virtio_pci_cap *out)
{

    volatile uint8_t *p = ecam_ptr(0,2,0, cap_off);
    struct virtio_pci_cap c;

    c.cap_vndr = p[0];
    c.cap_next = p[1] & 0xFC;      // dword-aligned
    c.cap_len  = p[2];
    c.cfg_type = p[3];
    c.bar      = p[4];
    c.id       = p[5];
    c.offset   = *(volatile uint32_t*)(p + 8);
    c.length   = *(volatile uint32_t*)(p + 12);

    // minimal sanity checks
    if (c.cap_vndr != 0x09) return false;               // not a vendor-specific cap
    if (c.cap_len < sizeof(struct virtio_pci_cap)) return false;

    *out = c;
    return true;
}

void capability_list(volatile uint8_t *p, uint8_t i){
    put_uart("\n", 0);
    put_uart("Capability     ", 0);
    send_value_dec(i);

    struct virtio_pci_cap *cap = (struct virtio_pci_cap*)p;
    put_uart("Capability id: ", 0);
    send_value_hex(&cap->cap_vndr, sizeof(cap->cap_vndr));
    put_uart("next:          ", 0);
    send_value_hex(&cap->cap_next, sizeof(cap->cap_next));
    put_uart("bar:           ", 0);
    send_value_hex(&cap->bar, sizeof(cap->bar));
    put_uart("config type:   ", 0);
    send_value_hex(&cap->cfg_type, sizeof(cap->cfg_type));
    put_uart("Offset:        ", 0);
    send_value_hex(&cap->offset, sizeof(cap->offset));
    if((cap->cap_next & 0xFC) != 0x0){
        volatile uint8_t *p = ecam_ptr(0,2,0, cap->cap_next & 0xFC);
        capability_list(p, ++i);
    }
    put_uart("\n", 0);
}

uint64_t BAR_size(uint8_t bar_index){
    volatile uint32_t *BAR = (volatile uint32_t*)ecam_ptr(0,2,0, 0x10);
    uint64_t size;
    volatile uint16_t *coomand_register = (volatile uint16_t*)ecam_ptr(0,2,0, 0x04);
    uint16_t coomand_register_before = *coomand_register;
    *coomand_register &= 0xFFFC;
    //send_value_hex(&val, sizeof(val));
    //Memory Space BAR with 32 bit
    if((BAR[bar_index] & 0x0000007) == 0){
        uint32_t BAR_value_before = BAR[bar_index];
        //send_value_hex(&BAR_value_before, sizeof(BAR_value_before));
        BAR[bar_index] = 0xFFFFFFFF;
        size = (uint64_t) ~(BAR[bar_index] & 0xFFFFFFF0)+1;
        BAR[bar_index] = BAR_value_before;
    }
    //Memory Space BAR with 64 bit
    else if((BAR[bar_index] & 0x0000007)==0x4){ 
        uint32_t BAR_value_before1 = BAR[bar_index];
        uint32_t BAR_value_before2 = BAR[bar_index+1];
        //send_value_hex(&BAR_value_before1, sizeof(BAR_value_before1));
        //send_value_hex(&BAR_value_before2, sizeof(BAR_value_before2));
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

uint64_t* allocate(uint8_t bar_index, uint8_t memory_type){
    //memory types:
    //1: IO    2: non prefetchable    3: prefetchable
    uint64_t bar_size = BAR_size(bar_index);
    uint64_t offset = bar_size;
    volatile uint32_t *BAR = (volatile uint32_t*)ecam_ptr(0,2,0, 0x10);
    //put_uart("BAR4 value:", 1);
    //send_value_hex(BAR+bar_index, sizeof(*(BAR+4)));
    if(memory_type == 1){
        if(CPU_PCI_IO_BASE_ADDRESS+offset+bar_size < CPU_PCI_IO_BASE_ADDRESS+CPU_PCI_IO_SIZE){

            BAR[bar_index] = (uint32_t) BUS_PCI_IO_BASE_ADDRESS+offset;
            return (uint_fast64_t*) CPU_PCI_IO_BASE_ADDRESS+offset;
        }
    }
    else if(memory_type == 2){
        if(CPU_PCI_MEMORY_NON_PREFETCHABLE_BASE+offset+bar_size < CPU_PCI_MEMORY_NON_PREFETCHABLE_BASE+CPU_PCI_MEMORY_NON_PREFETCHABLE_SIZE){
            BAR[bar_index] = (uint32_t) BUS_PCI_MEMORY_NON_PREFETCHABLE_BASE+offset;
            return (uint_fast64_t*) CPU_PCI_MEMORY_NON_PREFETCHABLE_BASE+offset;
        }
    }
    else if(memory_type == 3){
        if(CPU_PCI_MEMORY_PREFETCHABLE_BASE+offset+bar_size < CPU_PCI_MEMORY_PREFETCHABLE_BASE+CPU_PCI_MEMORY_PREFETCHABLE_SIZE){

            uint64_t v = (uint64_t) CPU_PCI_MEMORY_PREFETCHABLE_BASE+offset;
            // put_uart("New BAR:", 1);
            // send_value_hex(&v, sizeof(v));
            BAR[bar_index] = (uint32_t) v;
            BAR[bar_index+1] = (uint32_t) (v>>32);
            // put_uart("New BAR4:", 1);
            // send_value_hex(&BAR[bar_index], sizeof(BAR[bar_index]));
            // put_uart("New BAR5:", 1);
            // send_value_hex(&BAR[bar_index+1], sizeof(BAR[bar_index+1]));
            return (uint_fast64_t*) v;
        }
    }
    else{
        put_uart("Wrong memory type.", 1);
        return NULL;
    }

}


void init_pci(uint64_t *bar4){
    volatile uint16_t *coomand_register = (volatile uint16_t*)ecam_ptr(0,2,0, 0x04);
    uint16_t coomand_register_before = *coomand_register;
    *coomand_register |= 0x0003;
    volatile struct virtio_pci_common_cfg *common_cfg = (volatile struct virtio_pci_common_cfg*) bar4;
    common_cfg->device_status = 0;
    common_cfg->device_status |= 1;
    common_cfg->device_status |= 2;

    common_cfg->device_feature_select = 0x0;
    uint32_t features0 = common_cfg->device_feature;
    put_uart("feature0: ", 0);
    send_value_hex(&features0, sizeof(features0));
    common_cfg->driver_feature_select = features0 & 0x0;
    common_cfg->driver_feature = 0x0;

    common_cfg->device_feature_select = 0x1;
    uint32_t features1 = common_cfg->device_feature;
    put_uart("feature1: ", 0);
    send_value_hex(&features1, sizeof(features1));
    common_cfg->driver_feature_select = features1 & 0x1;
    common_cfg->driver_feature = 0x1;
    common_cfg->device_status |= 8;
    uint32_t device_status = common_cfg->device_status;
    if(device_status == 0x0b){
        put_uart("Device status is ok: ", 1);
        send_value_hex(&device_status, sizeof(device_status));
    }
    put_uart("Number of queues: ", 0);
    send_value_hex(&(common_cfg->num_queues), sizeof(common_cfg->num_queues));

    common_cfg->queue_select = 0;
    
    uint16_t max_queue_size = common_cfg->queue_size;
    common_cfg->queue_size = max_queue_size;
    put_uart("Queue size: ", 0);
    send_value_hex(&(common_cfg->queue_size), sizeof(common_cfg->queue_size));
    
    
    struct virtq virtq = construct_virtqueue(common_cfg);
    common_cfg->device_status |= 4;
    
    put_uart("Device status after init: ", 0);
    send_value_hex(&(common_cfg->device_status), sizeof(common_cfg->device_status));
    
    //##############################################################################
    struct virtq_desc *d = virtq.desc;
    struct virtq_avail *virtq_avail = virtq.avail;
    struct virtq_desc *response = d+1;
    struct virtio_gpu_ctrl_hdr *b = (struct virtio_gpu_ctrl_hdr *) alloc(sizeof(struct virtio_gpu_ctrl_hdr), 8);   // make sure d points to valid storage!
    struct virtio_gpu_resp_display_info *info = (struct virtio_gpu_resp_display_info *) alloc(sizeof(struct virtio_gpu_resp_display_info), 8);
    d->addr = (uint64_t) b;
    d->len = sizeof(*b);
    d->flags = VIRTQ_DESC_F_NEXT;
    d->next = 1;

    response->addr = (uint64_t) info;
    response->len = sizeof(*info);
    response->flags = VIRTQ_DESC_F_WRITE;
    response->next = 0;
    b->type    = VIRTIO_GPU_CMD_GET_DISPLAY_INFO;
    b->flags   = 0;
    b->fence_id = 0;
    b->ctx_id  = 0;
    
    virtq_avail->flags=0;
    virtq_avail->ring[0] = 0;
    virtq_avail->idx = 1;
    uint8_t cap_offset = get_capability_offset();
    volatile uint8_t *p = ecam_ptr(0,2,0, cap_offset);
    struct virtio_pci_notify_cap *notify_cap = get_notify_cap(p);
    put_uart("\n", 1);
    put_uart("queue_select ", 0);
    send_value_hex(&(common_cfg->queue_select), sizeof(common_cfg->queue_select));
    put_uart("cap offset: ", 0);
    send_value_hex(&(notify_cap->cap.offset), sizeof(notify_cap->cap.offset));
    put_uart("common_cfg->queue_notify_off: ", 0);
    send_value_hex(&(common_cfg->queue_notify_off), sizeof(common_cfg->queue_notify_off));
    put_uart("notify_cap->notify_off_multiplier: ", 0);
    send_value_hex(&(notify_cap->notify_off_multiplier), sizeof(notify_cap->notify_off_multiplier));
    volatile uint32_t *queue_notify_address = (uint32_t*)((volatile uint8_t*)bar4 + notify_cap->cap.offset + common_cfg->queue_notify_off * notify_cap->notify_off_multiplier);
    *queue_notify_address = 0;
    //##############################################################################
    

    struct virtq_used *virtq_used = virtq.used;
    put_uart("idx: ", 0);
    send_value_hex(&(virtq_used->idx), sizeof(virtq_used->idx));
    put_uart("test", 1);
    struct virtq_used_elem *used_elem = &(virtq_used->ring[0]);
    put_uart("id: ", 0);
    send_value_dec((int) used_elem->id);
    put_uart("len: ", 0);
    send_value_dec((int) used_elem->len);
    *coomand_register = coomand_register_before;

    struct virtio_gpu_display_one display_one = info->pmodes[0];
    struct virtio_gpu_rect rec = display_one.r;
    put_uart("x: ", 0);
    send_value_dec(rec.x);
    put_uart("y: ", 0);
    send_value_dec(rec.y);
    put_uart("width: ", 0);
    send_value_dec(rec.width);
    put_uart("height: ", 0);
    send_value_dec(rec.height);
}

struct virtq construct_virtqueue(volatile struct virtio_pci_common_cfg *common_cfg){

    init();
    put_uart("descriptor: ", 1);
    uint8_t* addr_descriptor_table = alloc(common_cfg->queue_size*16, 16);
    common_cfg->queue_desc = (uint64_t) addr_descriptor_table;


    put_uart("driver area: ", 1);
    uint8_t* addr_driver = alloc(common_cfg->queue_size*2+6, 2);
    common_cfg->queue_driver = (uint64_t) addr_driver;
    

    put_uart("driver area: ", 1);
    uint8_t* addr_device = alloc(common_cfg->queue_size*8+6, 4);
    common_cfg->queue_device = (uint64_t) addr_device;

    struct virtq virtq;
    virtq.desc = (struct virtq_desc *) addr_descriptor_table;
    virtq.avail = (struct virtq_avail *) addr_driver;
    virtq.used = (struct virtq_used *) addr_device;

    common_cfg->queue_enable = 1;
    
    return virtq;
}



void supply_buffer_to_device(struct virtq *virtq, volatile struct virtio_pci_common_cfg *common_cfg, uint64_t *bar4, struct virtio_gpu_resp_display_info* info0){
    
    struct virtq_desc *d = virtq->desc;
    struct virtq_avail *virtq_avail = virtq->avail;
    struct virtq_desc *response = d+1;
    struct virtio_gpu_ctrl_hdr *b = (struct virtio_gpu_ctrl_hdr *) alloc(sizeof(struct virtio_gpu_ctrl_hdr), 8);   // make sure d points to valid storage!
    struct virtio_gpu_resp_display_info *info = (struct virtio_gpu_resp_display_info *) alloc(sizeof(struct virtio_gpu_resp_display_info), 8);
    memcpy(info0, info, sizeof(*info));
    d->addr = (uint64_t) b;
    d->len = sizeof(*b);
    d->flags = VIRTQ_DESC_F_NEXT;
    d->next = 1;

    response->addr = (uint64_t) info;
    response->len = sizeof(*info);
    response->flags = VIRTQ_DESC_F_WRITE;
    response->next = 0;
    b->type    = VIRTIO_GPU_CMD_GET_DISPLAY_INFO;
    b->flags   = 0;
    b->fence_id = 0;
    b->ctx_id  = 0;
    
    virtq_avail->flags=0;
    virtq_avail->ring[0] = 0;
    virtq_avail->idx = 1;
    uint8_t cap_offset = get_capability_offset();
    volatile uint8_t *p = ecam_ptr(0,2,0, cap_offset);
    struct virtio_pci_notify_cap *notify_cap = get_notify_cap(p);
    put_uart("\n", 1);
    put_uart("queue_select ", 0);
    send_value_hex(&(common_cfg->queue_select), sizeof(common_cfg->queue_select));
    put_uart("cap offset: ", 0);
    send_value_hex(&(notify_cap->cap.offset), sizeof(notify_cap->cap.offset));
    put_uart("common_cfg->queue_notify_off: ", 0);
    send_value_hex(&(common_cfg->queue_notify_off), sizeof(common_cfg->queue_notify_off));
    put_uart("notify_cap->notify_off_multiplier: ", 0);
    send_value_hex(&(notify_cap->notify_off_multiplier), sizeof(notify_cap->notify_off_multiplier));
    volatile uint32_t *queue_notify_address = (uint32_t*)((volatile uint8_t*)bar4 + notify_cap->cap.offset + common_cfg->queue_notify_off * notify_cap->notify_off_multiplier);
    *queue_notify_address = 0;

    
}

struct virtio_pci_notify_cap* get_notify_cap(volatile uint8_t *p){
    volatile struct virtio_pci_cap *cap = (struct virtio_pci_cap*)p;
    while((cap->cap_next & 0xFC) != 0x0){
        if(cap->cfg_type == 2){
            struct virtio_pci_notify_cap *notify_cap = (struct virtio_pci_notify_cap *) cap;
            return notify_cap;
        }
        volatile uint8_t *p = ecam_ptr(0,2,0, cap->cap_next & 0xFC);
        cap = (volatile struct virtio_pci_cap*)p;
        
    }
    return NULL;
}