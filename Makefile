CC      = aarch64-elf-gcc
OBJCOPY = aarch64-elf-objcopy

CFLAGS  = -ffreestanding -mgeneral-regs-only
LDFLAGS = -nostdlib -nostartfiles -T linker.ld

CXX = aarch64-elf-g++

CXXFLAGS = $(CFLAGS)

OBJS = boot.o kernel.o uart.o allocator.o virtio_gpu_driver.o pci_driver.o

all: kernel.img

boot.o: boot.S
	$(CC) $(CFLAGS) -c $< -o $@

kernel.o: kernel.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

virtio_gpu_driver.o: virtio_gpu_driver.cpp virtio_gpu_driver.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

pci_driver.o: pci_driver.c pci_driver.h
	$(CC) $(CFLAGS) -c $< -o $@

uart.o: uart.c uart.h
	$(CC) $(CFLAGS) -c $< -o $@

allocator.o: allocator.c allocator.h
	$(CC) $(CFLAGS) -c $< -o $@ 

kernel.elf: linker.ld $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

kernel.img: kernel.elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -f $(OBJS) kernel.elf kernel.img
