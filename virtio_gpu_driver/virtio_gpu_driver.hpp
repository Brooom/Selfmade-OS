#ifndef VIRTIO_GPU_DRIVER_HPP
#define VIRTIO_GPU_DRIVER_HPP

#include <stdint.h>
#include <stddef.h>
#include "../pci_driver.h"


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

struct pixelcolor{
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;
};

/* Rectangle (x, y, width, height) */
struct virtio_gpu_rect {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
};

class virtio_gpu_driver{
    public:
        struct bus_device_function bdf;
        virtio_gpu_driver();
        void init_2D_frame_buffer();
        uint8_t* connect_resource_to_memory();
        void set_scanout_param();
        void transfer_to_host_2d();
        void resource_flush();
        void draw_pixel(int x, int y, struct pixelcolor);
        void draw_rec(struct virtio_gpu_rect rec, pixelcolor c);
        void draw_letter(int x, int y, char l, struct pixelcolor c, int size);
        void draw_text(int x, int y, const char *string, int string_size, struct pixelcolor c, int text_size);

    private:
        struct virtq virtq;
        int queue_size = 0;
        uint16_t descriptor_index = 0;
        volatile uint32_t *queue_notify_address;
        int screen_height = 800;
        int screen_width = 1280;
        pixelcolor* display;
        struct virtq construct_virtqueue(volatile struct virtio_pci_common_cfg *common_cfg);
        virtq_desc* get_next_descriptor();
        

};
#endif