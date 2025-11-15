#include "virtio_gpu_driver.hpp"
#include "kernel_logs/kernel_logger.hpp"
#include "../allocator.h"
#include "letters/font8x8_basic.h"

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

enum virtio_gpu_formats {
    VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM = 1,
    VIRTIO_GPU_FORMAT_B8G8R8X8_UNORM = 2,
    VIRTIO_GPU_FORMAT_A8R8G8B8_UNORM = 3,
    VIRTIO_GPU_FORMAT_X8R8G8B8_UNORM = 4,

    VIRTIO_GPU_FORMAT_R8G8B8A8_UNORM = 67,
    VIRTIO_GPU_FORMAT_X8B8G8R8_UNORM = 68,

    VIRTIO_GPU_FORMAT_A8B8G8R8_UNORM = 121,
    VIRTIO_GPU_FORMAT_R8G8B8X8_UNORM = 134,
};

struct virtio_gpu_ctrl_hdr {
    uint32_t type;      /* enum virtio_gpu_ctrl_type */
    uint32_t flags;     /* VIRTIO_GPU_FLAG_* */
    uint64_t fence_id;  /* optional fence value when FENCE flag set */
    uint32_t ctx_id;    /* 3D context identifier (for 3D commands) */
    uint8_t ring_idx;  /* control vs cursor queue (if device supports multiple) */
    uint8_t padding[3];
};

struct virtio_gpu_resource_attach_backing {
    struct virtio_gpu_ctrl_hdr hdr;
    uint32_t resource_id;
    uint32_t nr_entries;
};

struct virtio_gpu_mem_entry {
    uint64_t addr;
    uint32_t length;
    uint32_t padding;
};


#define VIRTQ_DESC_F_NEXT 1 /* This marks a buffer as continuing. */
#define VIRTQ_DESC_F_WRITE 2 /* This marks a descriptor as device write-only (otherwise device read-only). */
#define VIRTIO_GPU_MAX_SCANOUTS 16



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

struct virtio_gpu_resource_create_2d {
    struct virtio_gpu_ctrl_hdr hdr;
    uint32_t resource_id;
    uint32_t format;   /* one of enum virtio_gpu_formats */
    uint32_t width;
    uint32_t height;
};

struct virtio_gpu_set_scanout {
    struct virtio_gpu_ctrl_hdr hdr;   // type = VIRTIO_GPU_CMD_SET_SCANOUT
    struct virtio_gpu_rect r;         // { x, y, width, height } in the resource
    uint32_t scanout_id;                  // which display head (usually 0)
    uint32_t resource_id;                 // the resource to scan out; 0 = detach
};

struct virtio_gpu_transfer_to_host_2d {
    struct virtio_gpu_ctrl_hdr hdr;
    struct virtio_gpu_rect r;
    uint64_t offset;
    uint32_t resource_id;
    uint32_t padding;
};

struct virtio_gpu_resource_flush {
    struct virtio_gpu_ctrl_hdr hdr;
    struct virtio_gpu_rect r;
    uint32_t resource_id;
    uint32_t padding;
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
    kernel_logger::log("Available feature0: %x", features0);
    common_cfg->driver_feature_select = features0 & 0x0;
    common_cfg->driver_feature = 0x0;
    common_cfg->device_feature_select = 0x1;
    uint32_t features1 = common_cfg->device_feature;
    kernel_logger::log("Available feature1: %x", features1);
    common_cfg->driver_feature_select = features1 & 0x1;
    common_cfg->driver_feature = 0x1;

    common_cfg->device_status |= 8;
    // Check if everything worked properly
    uint32_t device_status = common_cfg->device_status;
    if(device_status == 0x0b){
        kernel_logger::log("Device status is ok");
    }
    kernel_logger::log("Number of queues: %x", common_cfg->num_queues);

    // Initialize queues
    common_cfg->queue_select = 0;
    uint16_t max_queue_size = common_cfg->queue_size;
    common_cfg->queue_size = max_queue_size;
    kernel_logger::log("Queue size: %x", common_cfg->queue_size);

    virtq = construct_virtqueue(common_cfg);

    common_cfg->device_status |= 4;
    
    kernel_logger::log("Device status after init: %x", common_cfg->device_status);
    
    // GPU specific initalizations
    volatile struct virtq_desc *d = get_next_descriptor();
    volatile struct virtq_avail *virtq_avail = virtq.avail;
    volatile struct virtq_desc *response = get_next_descriptor();
    volatile struct virtio_gpu_ctrl_hdr *b = 
        (struct virtio_gpu_ctrl_hdr *) alloc(sizeof(struct virtio_gpu_ctrl_hdr), 8);
    struct virtio_gpu_resp_display_info *info = 
        (struct virtio_gpu_resp_display_info *) alloc(sizeof(struct virtio_gpu_resp_display_info), 8);
    
    d->addr = (uint64_t) b;
    d->len = sizeof(*b);
    d->flags = VIRTQ_DESC_F_NEXT;
    d->next = 1;

    response->addr  = (uint64_t) info;
    response->len   = sizeof(*info);
    response->flags = VIRTQ_DESC_F_WRITE;
    response->next  = 0;

    b->type     = VIRTIO_GPU_CMD_GET_DISPLAY_INFO;
    b->flags    = 0;
    b->fence_id = 0;
    b->ctx_id   = 0;
    virtq_avail->flags      = 0;
    virtq_avail->ring[0]    = 0;
    virtq_avail->idx        = 1;
    volatile struct virtio_pci_notify_cap *notify_cap = (struct virtio_pci_notify_cap *) get_pci_capability(bdf, 2, 1);
    kernel_logger::log("#########");
    kernel_logger::log("%x", notify_cap->cap.bar);
    kernel_logger::log("\n");
    kernel_logger::log("queue_select %x", common_cfg->queue_select);
    kernel_logger::log("cap offset: %x", notify_cap->cap.offset);
    kernel_logger::log("common_cfg->queue_notify_off: %x", common_cfg->queue_notify_off);
    kernel_logger::log("notify_cap->notify_off_multiplier: %x", notify_cap->notify_off_multiplier);
    queue_notify_address = (uint32_t*)((volatile uint8_t*)bar4 + notify_cap->cap.offset + common_cfg->queue_notify_off * notify_cap->notify_off_multiplier);
    
    *queue_notify_address = 0;
    //##############################################################################
    
    volatile struct virtq_used *virtq_used = virtq.used;
    while(virtq_used->idx==0){}
    kernel_logger::log("idx: %x", virtq_used->idx);
    kernel_logger::log("test");
    volatile struct virtq_used_elem *used_elem = &(virtq_used->ring[0]);
    kernel_logger::log("id: %u", (int) used_elem->id);
    kernel_logger::log("len: ", (int) used_elem->len);
    //*comand_register = comand_register_before;
    struct virtio_gpu_resp_display_info::virtio_gpu_display_one display_one = info->pmodes[0];
    volatile struct virtio_gpu_rect rec = display_one.r;
    
    kernel_logger::log("x: %u", rec.x);
    kernel_logger::log("y: %u", rec.y);
    kernel_logger::log("width: %u", rec.width);
    kernel_logger::log("height: ", rec.height);
}

struct virtq virtio_gpu_driver::construct_virtqueue(volatile struct virtio_pci_common_cfg *common_cfg){
    queue_size = common_cfg->queue_size;

    kernel_logger::log("descriptor: ");
    uint8_t* addr_descriptor_table = alloc(queue_size*16, 16);
    common_cfg->queue_desc = (uint64_t) addr_descriptor_table;


    kernel_logger::log("driver area: ");
    uint8_t* addr_driver = alloc(queue_size*2+6, 2);
    common_cfg->queue_driver = (uint64_t) addr_driver;
    

    kernel_logger::log("device area: ");
    uint8_t* addr_device = alloc(queue_size*8+6, 4);
    common_cfg->queue_device = (uint64_t) addr_device;

    struct virtq virtq;
    virtq.desc = (struct virtq_desc *) addr_descriptor_table;
    virtq.avail = (struct virtq_avail *) addr_driver;
    virtq.used = (struct virtq_used *) addr_device;

    common_cfg->queue_enable = 1;
    return virtq;
}

void virtio_gpu_driver::init_2D_frame_buffer(){
    volatile struct virtq_used *virtq_used = virtq.used;
    int descriptor_head_idx = descriptor_index;
    volatile struct virtq_desc *d0 = get_next_descriptor();
    int d1_idx = descriptor_index;
    volatile struct virtq_desc *d1 = get_next_descriptor();
    volatile struct virtq_avail *virtq_avail = virtq.avail;
    //Define gpu 2d resource
    volatile struct virtio_gpu_resource_create_2d  *gpu_2d_resource = 
        (struct virtio_gpu_resource_create_2d  *) 
        alloc(sizeof(struct virtio_gpu_resource_create_2d ), 8);

    volatile struct virtio_gpu_ctrl_hdr *response_gpu_2d_resource = 
        (struct virtio_gpu_ctrl_hdr *) alloc(sizeof(struct virtio_gpu_ctrl_hdr), 8);
    
    gpu_2d_resource->hdr.type       = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D;
    gpu_2d_resource->hdr.flags      = 0;
    gpu_2d_resource->hdr.fence_id   = 0;
    gpu_2d_resource->hdr.ctx_id     = 0;
    gpu_2d_resource->format         = VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM;
    gpu_2d_resource->height         = 800;
    gpu_2d_resource->width          = 1280;
    gpu_2d_resource->resource_id    = 1;
    //Connect descriptor entry with gpu 2d resource
    d0->addr     = (uint64_t) gpu_2d_resource;
    d0->len      = sizeof(*gpu_2d_resource);
    d0->flags    = VIRTQ_DESC_F_NEXT;
    d0->next     = d1_idx;
    
    //Connect descriptor entry with gpu 2d resource response
    d1->addr     = (uint64_t) response_gpu_2d_resource;
    d1->len      = sizeof(*response_gpu_2d_resource);
    d1->flags    = VIRTQ_DESC_F_WRITE;
    d1->next     = 0;
    int used_idx = virtq_used->idx;
    virtq_avail->ring[(virtq_avail->idx) % queue_size] = descriptor_head_idx;
    virtq_avail->idx = virtq_avail->idx + 1;
    *queue_notify_address = 0;
    while(virtq_used->idx == used_idx){};
    if(response_gpu_2d_resource->type != VIRTIO_GPU_RESP_OK_NODATA){
        kernel_logger::log("Wrong return typ for resource: %x", response_gpu_2d_resource->type);
    }
    else{
        kernel_logger::log("Created 2d resource.");
    }
}

virtq_desc* virtio_gpu_driver::get_next_descriptor(){
    virtq_desc* returnvalue = virtq.desc + descriptor_index;
    descriptor_index = (descriptor_index+1) % queue_size;
    return returnvalue;
}

uint8_t* virtio_gpu_driver::connect_resource_to_memory(){
    volatile struct virtq_used *virtq_used = virtq.used;
    int descriptor_head_idx = descriptor_index;
    volatile struct virtq_desc *d0 = get_next_descriptor();
    int d1_idx = descriptor_index;
    volatile struct virtq_desc *d1 = get_next_descriptor();
    int d2_idx = descriptor_index;
    volatile struct virtq_desc *d2 = get_next_descriptor();
    volatile struct virtq_avail *virtq_avail = virtq.avail;

    //Define virtio_gpu_resource_attach_backing
    volatile struct virtio_gpu_resource_attach_backing  *resource_attach_backing  = 
        (struct virtio_gpu_resource_attach_backing  *) 
        alloc(sizeof(struct virtio_gpu_resource_attach_backing), 8);

    //Define virtio_gpu_mem_entry
    volatile struct virtio_gpu_mem_entry *mem_entry = 
        (struct virtio_gpu_mem_entry *) 
        alloc(sizeof(struct virtio_gpu_mem_entry), 8);

    volatile struct virtio_gpu_ctrl_hdr *response = 
        (struct virtio_gpu_ctrl_hdr *) 
        alloc(sizeof(struct virtio_gpu_ctrl_hdr), 8);
    
    resource_attach_backing->hdr.type       = VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING;
    resource_attach_backing->hdr.flags      = 0;
    resource_attach_backing->hdr.fence_id   = 0;
    resource_attach_backing->hdr.ctx_id     = 0;
    resource_attach_backing->resource_id    = 1;
    resource_attach_backing->nr_entries     = 1;
    //Connect descriptor entry with resource_attach_backing
    d0->addr     = (uint64_t) resource_attach_backing;
    d0->len      = sizeof(*resource_attach_backing);
    d0->flags    = VIRTQ_DESC_F_NEXT;
    d0->next     = d1_idx;
    uint8_t *memory_2d = alloc(800*1280*4, 8);
    display = (pixelcolor *) memory_2d;
    mem_entry->addr     = (uint64_t) memory_2d;
    mem_entry->length   = 800*1280*4;
    mem_entry->padding  = 0;
    //Connect descriptor entry with mem_entry
    d1->addr     = (uint64_t) mem_entry;
    d1->len      = sizeof(*mem_entry);
    d1->flags    = VIRTQ_DESC_F_NEXT;
    d1->next     = d2_idx;
    int used_idx = virtq_used->idx;

    d2->addr     = (uint64_t) response;
    d2->len      = sizeof(*response);
    d2->flags    = VIRTQ_DESC_F_WRITE;
    d2->next     = 0;


    virtq_avail->ring[(virtq_avail->idx) % queue_size] = descriptor_head_idx;
    virtq_avail->idx = virtq_avail->idx + 1;
    *queue_notify_address = 0;
    while(virtq_used->idx == used_idx){};
    if(response->type != VIRTIO_GPU_RESP_OK_NODATA){
        kernel_logger::log("Connection of resource and memory did not work: %x", response->type);
    }
    else{
        kernel_logger::log("Connected resource and memory.");
    }
    return memory_2d;
}

void virtio_gpu_driver::set_scanout_param(){
    volatile struct virtq_used *virtq_used = virtq.used;
    int descriptor_head_idx = descriptor_index;
    volatile struct virtq_desc *d0 = get_next_descriptor();
    int d1_idx = descriptor_index;
    volatile struct virtq_desc *d1 = get_next_descriptor();
    volatile struct virtq_avail *virtq_avail = virtq.avail;

    volatile struct virtio_gpu_set_scanout *gpu_cmd_set_scan_out = 
        (struct virtio_gpu_set_scanout *) 
        alloc(sizeof(struct virtio_gpu_set_scanout ), 8);

    volatile struct virtio_gpu_ctrl_hdr *response = 
        (struct virtio_gpu_ctrl_hdr *) alloc(sizeof(struct virtio_gpu_ctrl_hdr), 8);
    
    gpu_cmd_set_scan_out->hdr.type       = VIRTIO_GPU_CMD_SET_SCANOUT;
    gpu_cmd_set_scan_out->hdr.flags      = 0;
    gpu_cmd_set_scan_out->hdr.fence_id   = 0;
    gpu_cmd_set_scan_out->hdr.ctx_id     = 0;
    gpu_cmd_set_scan_out->r.x = 0;
    gpu_cmd_set_scan_out->r.y = 0;
    gpu_cmd_set_scan_out->r.width = 1280;
    gpu_cmd_set_scan_out->r.height = 800;
    gpu_cmd_set_scan_out->resource_id = 1;
    gpu_cmd_set_scan_out->scanout_id = 0;

    d0->addr     = (uint64_t) gpu_cmd_set_scan_out;
    d0->len      = sizeof(*gpu_cmd_set_scan_out);
    d0->flags    = VIRTQ_DESC_F_NEXT;
    d0->next     = d1_idx;
    
    d1->addr     = (uint64_t) response;
    d1->len      = sizeof(*response);
    d1->flags    = VIRTQ_DESC_F_WRITE;
    d1->next     = 0;
    int used_idx = virtq_used->idx;
    virtq_avail->ring[(virtq_avail->idx) % queue_size] = descriptor_head_idx;
    virtq_avail->idx = virtq_avail->idx + 1;
    *queue_notify_address = 0;
    while(virtq_used->idx == used_idx){};
    if(response->type != VIRTIO_GPU_RESP_OK_NODATA){
        kernel_logger::log("Wrong return typ set scanout param: %x", response->type);
    }
    else{
        kernel_logger::log("Set scanout parameters.", 1);
    }
}

void virtio_gpu_driver::transfer_to_host_2d(){
    volatile struct virtq_used *virtq_used = virtq.used;
    int descriptor_head_idx = descriptor_index;
    volatile struct virtq_desc *d0 = get_next_descriptor();
    int d1_idx = descriptor_index;
    volatile struct virtq_desc *d1 = get_next_descriptor();
    volatile struct virtq_avail *virtq_avail = virtq.avail;

    volatile struct virtio_gpu_transfer_to_host_2d *transfer_to_host_2d = 
        (struct virtio_gpu_transfer_to_host_2d *) 
        alloc(sizeof(struct virtio_gpu_transfer_to_host_2d), 8);

    volatile struct virtio_gpu_ctrl_hdr *response = 
        (struct virtio_gpu_ctrl_hdr *) alloc(sizeof(struct virtio_gpu_ctrl_hdr), 8);
    
    transfer_to_host_2d->hdr.type       = VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D;
    transfer_to_host_2d->hdr.flags      = 0;
    transfer_to_host_2d->hdr.fence_id   = 0;
    transfer_to_host_2d->hdr.ctx_id     = 0;
    transfer_to_host_2d->offset         = 0;
    transfer_to_host_2d->padding        = 0;
    transfer_to_host_2d->r.x            = 0;
    transfer_to_host_2d->r.y            = 0;
    transfer_to_host_2d->r.width        = 1280;
    transfer_to_host_2d->r.height       = 800;
    transfer_to_host_2d->resource_id    = 1;

    d0->addr     = (uint64_t) transfer_to_host_2d;
    d0->len      = sizeof(*transfer_to_host_2d);
    d0->flags    = VIRTQ_DESC_F_NEXT;
    d0->next     = d1_idx;
    
    d1->addr     = (uint64_t) response;
    d1->len      = sizeof(*response);
    d1->flags    = VIRTQ_DESC_F_WRITE;
    d1->next     = 0;

    int used_idx = virtq_used->idx;
    virtq_avail->ring[(virtq_avail->idx) % queue_size] = descriptor_head_idx;
    virtq_avail->idx = virtq_avail->idx + 1;
    *queue_notify_address = 0;
    while(virtq_used->idx == used_idx){};
    if(response->type != VIRTIO_GPU_RESP_OK_NODATA){
        kernel_logger::log("Wrong return typ for transfer: %x", false ,response->type);
    }
    else{
        kernel_logger::log("Transfer to host worked.", false);
    }
}

void virtio_gpu_driver::resource_flush(){
    volatile struct virtq_used *virtq_used = virtq.used;
    int descriptor_head_idx = descriptor_index;
    volatile struct virtq_desc *d0 = get_next_descriptor();
    int d1_idx = descriptor_index;
    volatile struct virtq_desc *d1 = get_next_descriptor();
    volatile struct virtq_avail *virtq_avail = virtq.avail;

    volatile struct virtio_gpu_resource_flush *resource_flush = 
        (struct virtio_gpu_resource_flush *) 
        alloc(sizeof(struct virtio_gpu_resource_flush), 8);

    volatile struct virtio_gpu_ctrl_hdr *response = 
        (struct virtio_gpu_ctrl_hdr *) alloc(sizeof(struct virtio_gpu_ctrl_hdr), 8);
    
    resource_flush->hdr.type       = VIRTIO_GPU_CMD_RESOURCE_FLUSH;
    resource_flush->hdr.flags      = 0;
    resource_flush->hdr.fence_id   = 0;
    resource_flush->hdr.ctx_id     = 0;
    resource_flush->padding        = 0;
    resource_flush->r.x            = 0;
    resource_flush->r.y            = 0;
    resource_flush->r.width        = 1280;
    resource_flush->r.height       = 800;
    resource_flush->resource_id    = 1;

    d0->addr     = (uint64_t) resource_flush;
    d0->len      = sizeof(*resource_flush);
    d0->flags    = VIRTQ_DESC_F_NEXT;
    d0->next     = d1_idx;
    
    d1->addr     = (uint64_t) response;
    d1->len      = sizeof(*response);
    d1->flags    = VIRTQ_DESC_F_WRITE;
    d1->next     = 0;

    int used_idx = virtq_used->idx;
    virtq_avail->ring[(virtq_avail->idx) % queue_size] = descriptor_head_idx;
    virtq_avail->idx = virtq_avail->idx + 1;
    *queue_notify_address = 0;
    while(virtq_used->idx == used_idx){};
    if(response->type != VIRTIO_GPU_RESP_OK_NODATA){
        kernel_logger::log("Wrong return typ for resource flush: ", false, response->type);
    }
    else{
        kernel_logger::log("Resource flush worked.", false);
    }
}


void virtio_gpu_driver::draw_pixel(int x, int y, struct pixelcolor c){
    display[y*screen_width+x] = c;
}

void virtio_gpu_driver::draw_rec(struct virtio_gpu_rect rec, struct pixelcolor c){
    int start_pos = rec.y*screen_width+rec.x;
    for(int i = 0; i<rec.width; ++i){
        for(int j = 0; j<rec.height; ++j){
            display[start_pos+i+(j*screen_width)] = c;
        }
    }
}

void virtio_gpu_driver::draw_letter(int x, int y, char l, struct pixelcolor c, int size){
    const char *letter = font8x8_basic[l];
    for(int i = 0; i<8*size; ++i){
        for (int j = 0; j < 8*size; ++j) {
            unsigned int bit = (letter[(unsigned int)(i/size)] >> (unsigned int)(j/size)) & 1u;  // i-th bit (0 = LSB)
            if(bit == 1){
                draw_pixel(x+j, y+i, c);
            }
            else{
                draw_pixel(x+j, y+i, (struct pixelcolor){0,0,0,255});
            }
        }
    }
}

void virtio_gpu_driver::draw_text(int x, int y, const char *string, int string_size, struct pixelcolor c, int text_size){
    for(int i = 0; i<string_size; ++i){
        draw_letter(x+i*text_size*8, y, string[i], c, text_size);
    }
}

void virtio_gpu_driver::clear_screen(){
    for(int i = 0; i<screen_width; ++i){
        for(int j = 0; j<screen_height; ++j){
            draw_pixel(i, j, pixelcolor{0, 0, 0, 255});
        }
    }
}