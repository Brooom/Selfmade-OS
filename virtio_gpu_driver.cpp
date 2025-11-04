#include "virtio_gpu_driver.hpp"
#include "uart.h"
#include "allocator.h"

struct virtio_pci_common_cfg {
    /* About the whole device. */
    uint32_t device_feature_select;   /* read-write */
    uint32_t device_feature;          /* read-only for driver */
    uint32_t driver_feature_select;   /* read-write */
    uint32_t driver_feature;          /* read-write */
    uint16_t config_msix_vector;      /* read-write */
    uint16_t num_queues;              /* read-only for driver */
    uint8_t  device_status;           /* read-write */
    uint8_t  config_generation;       /* read-only for driver */

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


virtio_gpu_driver::virtio_gpu_driver(){
    bdf = search_pci_device(0x1050, 0x1af4, 1);
    print_pci_config_space_header(bdf);

    volatile uint16_t *comand_register = (volatile uint16_t*)ecam_ptr(bdf, 0x04);
    volatile uint16_t comand_register_before = *comand_register;
    *comand_register |= 0x0007; // Set comand register bits such that the common config can be edited
    volatile struct virtio_pci_cap *common_conf_cap = (struct virtio_pci_cap *) get_pci_capability(bdf, 1, 1);
    volatile uint64_t *bar4 = alloc_bar_memory(bdf, common_conf_cap->bar, 1);
    volatile struct virtio_pci_common_cfg *common_cfg = (volatile struct virtio_pci_common_cfg*) bar4;
    
    //start initalization
    common_cfg->device_status = 0;
    common_cfg->device_status |= 1;
    common_cfg->device_status |= 2;

    // negotiate feature bits
    common_cfg->device_feature_select = 0x0;
    uint32_t features0 = common_cfg->device_feature;
    put_uart("Available feature0: ", 0);
    uart_send_hex_value(&features0, sizeof(features0));
    common_cfg->driver_feature_select = features0 & 0x0;
    common_cfg->driver_feature = 0x0;
    common_cfg->device_feature_select = 0x1;
    uint32_t features1 = common_cfg->device_feature;
    put_uart("Available feature1: ", 0);
    uart_send_hex_value(&features1, sizeof(features1));
    common_cfg->driver_feature_select = features1 & 0x1;
    common_cfg->driver_feature = 0x1;

    common_cfg->device_status |= 8;
    // Check if everything worked properly
    uint32_t device_status = common_cfg->device_status;
    if(device_status == 0x0b){
        put_uart("Device status is ok: ", 0);
    }
    put_uart("Number of queues: ", 0);
    uart_send_hex_value(&(common_cfg->num_queues), sizeof(common_cfg->num_queues));

    // Initialize queues
    common_cfg->queue_select = 0;
    uint16_t max_queue_size = common_cfg->queue_size;
    common_cfg->queue_size = max_queue_size;
    put_uart("Queue size: ", 0);
    uart_send_hex_value(&(common_cfg->queue_size), sizeof(common_cfg->queue_size));

    virtq = construct_virtqueue(common_cfg);

    common_cfg->device_status |= 4;
    
    put_uart("Device status after init: ", 0);
    uart_send_hex_value(&(common_cfg->device_status), sizeof(common_cfg->device_status));
    
    // GPU specific initalizations
    volatile struct virtq_desc *d = virtq.desc;
    volatile struct virtq_avail *virtq_avail = virtq.avail;
    volatile struct virtq_desc *response = d+1;
    volatile struct virtio_gpu_ctrl_hdr *b = (struct virtio_gpu_ctrl_hdr *) alloc(sizeof(struct virtio_gpu_ctrl_hdr), 8);   // make sure d points to valid storage!
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
    volatile struct virtio_pci_notify_cap *notify_cap = (struct virtio_pci_notify_cap *) get_pci_capability(bdf, 2, 1);
    put_uart("#########", 1);
    uart_send_hex_value(&(notify_cap->cap.bar), sizeof(notify_cap->cap.bar));
    put_uart("\n", 1);
    put_uart("queue_select ", 0);
    uart_send_hex_value(&(common_cfg->queue_select), sizeof(common_cfg->queue_select));
    put_uart("cap offset: ", 0);
    uart_send_hex_value(&(notify_cap->cap.offset), sizeof(notify_cap->cap.offset));
    put_uart("common_cfg->queue_notify_off: ", 0);
    uart_send_hex_value(&(common_cfg->queue_notify_off), sizeof(common_cfg->queue_notify_off));
    put_uart("notify_cap->notify_off_multiplier: ", 0);
    uart_send_hex_value(&(notify_cap->notify_off_multiplier), sizeof(notify_cap->notify_off_multiplier));
    volatile uint32_t *queue_notify_address = (uint32_t*)((volatile uint8_t*)bar4 + notify_cap->cap.offset + common_cfg->queue_notify_off * notify_cap->notify_off_multiplier);
    
    uint64_t addr = (uint64_t) queue_notify_address;
    *queue_notify_address = 0;
    //##############################################################################
    
    volatile struct virtq_used *virtq_used = virtq.used;
    //while(virtq_used->idx==0){}
    put_uart("idx: ", 0);
    uart_send_hex_value(&(virtq_used->idx), sizeof(virtq_used->idx));
    put_uart("test", 1);
    volatile struct virtq_used_elem *used_elem = &(virtq_used->ring[0]);
    put_uart("id: ", 0);
    uart_send_dec_value((int) used_elem->id);
    put_uart("len: ", 0);
    uart_send_dec_value((int) used_elem->len);
    *comand_register = comand_register_before;
    struct virtio_gpu_resp_display_info::virtio_gpu_display_one display_one = info->pmodes[0];
    volatile struct virtio_gpu_rect rec = display_one.r;
    
    put_uart("x: ", 0);
    uart_send_dec_value(rec.x);
    put_uart("y: ", 0);
    uart_send_dec_value(rec.y);
    put_uart("width: ", 0);
    uart_send_dec_value(rec.width);
    put_uart("height: ", 0);
    uart_send_dec_value(rec.height);
}

struct virtq virtio_gpu_driver::construct_virtqueue(volatile struct virtio_pci_common_cfg *common_cfg){

    put_uart("descriptor: ", 1);
    uint8_t* addr_descriptor_table = alloc(common_cfg->queue_size*16, 16);
    common_cfg->queue_desc = (uint64_t) addr_descriptor_table;


    put_uart("driver area: ", 1);
    uint8_t* addr_driver = alloc(common_cfg->queue_size*2+6, 2);
    common_cfg->queue_driver = (uint64_t) addr_driver;
    

    put_uart("device area: ", 1);
    uint8_t* addr_device = alloc(common_cfg->queue_size*8+6, 4);
    common_cfg->queue_device = (uint64_t) addr_device;

    struct virtq virtq;
    virtq.desc = (struct virtq_desc *) addr_descriptor_table;
    virtq.avail = (struct virtq_avail *) addr_driver;
    virtq.used = (struct virtq_used *) addr_device;

    common_cfg->queue_enable = 1;
    return virtq;
}
