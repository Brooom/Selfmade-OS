#include "std/args.h"
#include "terminal.hpp"
#include "allocator.h"
#include "std/formating.h"


terminal::terminal(virtio_gpu_driver *gd)
{
    gpu_driver = gd;
    circular_buffer = (char (*)[40][MAX_TEXT_LENGTH]) alloc(sizeof(char[40][MAX_TEXT_LENGTH]), 8);
    head = 0;
    tail = 0;
}

void terminal::printf(const char *fom, ...)
{
    va_list args;
    va_start(args, fom);
    format_string(fom, (*circular_buffer)[head], MAX_TEXT_LENGTH, args);
    va_end(args);
    head=(head+1)%40;
    if(((head - tail)%40) > 20){
        if(((head - tail)%40) > 21){
            //eror message
        }
        else{
            tail = (tail + 1) % 20;
        }
    }
    for(int i = 0; i<(head - tail)%40; i++){
        gpu_driver->draw_text(
            0, 
            i*4*8, 
            &((*circular_buffer)[(tail+i)%40][0]), 
            MAX_TEXT_LENGTH, 
            (struct pixelcolor){255,255,255,255}, 
            2
        );
    }
    gpu_driver->transfer_to_host_2d();
    gpu_driver->resource_flush();
}
