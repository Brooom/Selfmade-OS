#include "kernel_logs/kernel_logger.hpp"
#include "kernel_logs/kernel_logger.hpp"
#include "pci_driver.h"
#include "virtio_gpu_driver/virtio_gpu_driver.hpp"
#include "allocator/allocator.h"
#include "terminal.hpp"
#include "uart.h"
#include "terminal.hpp"
#include "uart.h"

extern "C" void kernel_main(void) {
    kernel_logger::log("Start kernel: %s %s", __DATE__, __TIME__);
    init_mmu();
    uint8_t *t = (uint8_t*) kalloc(10, 1);


    virtio_gpu_driver gpu_driver;
    gpu_driver.init_2D_frame_buffer();
    uint8_t* mem = gpu_driver.connect_resource_to_memory();
    gpu_driver.set_scanout_param();

    struct pixelcolor *p = (struct pixelcolor*) mem; 
    for(int i = 0; i<(1280); ++i){
        for(int j = 0; j<(800); ++j){
            gpu_driver.draw_pixel(i,j,(pixelcolor){0,0,0,255});
        }
    }
    gpu_driver.draw_rec((struct virtio_gpu_rect){100, 100, 100, 100}, (struct pixelcolor){0,0,255,255});
    // gpu_driver.draw_letter(0,0,'H',(struct pixelcolor){255,255,255,255}, 5);
    // gpu_driver.draw_letter(40,0,'a',(struct pixelcolor){255,255,255,255}, 5);
    // gpu_driver.draw_letter(80,0,'l',(struct pixelcolor){255,255,255,255}, 5);
    // gpu_driver.draw_letter(120,0,'l',(struct pixelcolor){255,255,255,255}, 5);
    // gpu_driver.draw_letter(160,0,'o',(struct pixelcolor){255,255,255,255}, 5);
    // char text[] = "hallo world";
    // gpu_driver.draw_text(0, 40, text, sizeof(text), (struct pixelcolor){255,255,255,255}, 2);

    gpu_driver.transfer_to_host_2d();
    gpu_driver.resource_flush();

    terminal te = terminal(&gpu_driver);
    uart_send("test1", 6);
    
    uart_send("test2", 6);
    te.printf("test");
    kernel_logger::init_terminal(&te);

    

    for(int i = 0; i<20; ++i){
        te.printf("hallo world");
    
    }

    for(int i = 0; i<3; ++i){
        te.printf("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
    
    }
    int d = 16;
    
    te.printf("int size %u", sizeof(int));
    te.printf("%x", 16);
    te.printf("test: %%", &d);

    kernel_logger::log("z");


    kernel_logger::log("z");

    
}
