#ifndef TERMINAL_H
#define TERMINAL_H

#include "virtio_gpu_driver/virtio_gpu_driver.hpp"
#define MAX_TEXT_LENGTH 80
#define NR_OF_LINES 20

class terminal{
    public:
        terminal(virtio_gpu_driver *gd);
        void printf(const char *fom, ...);
    
    private:
        char (*circular_buffer)[40][MAX_TEXT_LENGTH];
        int tail;
        int head;
        virtio_gpu_driver *gpu_driver;
};

#endif