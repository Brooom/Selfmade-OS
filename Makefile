CC      = aarch64-elf-gcc
OBJCOPY = aarch64-elf-objcopy

PROJECT_ROOT := $(CURDIR)

CFLAGS  = -ffreestanding -mgeneral-regs-only -I$(PROJECT_ROOT) -g3
LDFLAGS = -nostdlib -nostartfiles -T linker.ld 

CXX = aarch64-elf-g++
CXXFLAGS = $(CFLAGS) -I$(PROJECT_ROOT) -g3

BUILD_DIR = ./build

OBJS = boot.o kernel.o uart.o page_allocator.o virtio_gpu_driver.o \
       pci_driver.o terminal.o formating.o kernel_logger.o exceptions.o \
	   init_mmu.o mmu.o kernel_allocator.o

OBJ_FILES := $(addprefix $(BUILD_DIR)/,$(OBJS))

all: prepare $(BUILD_DIR)/kernel.img

prepare:
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/boot.o: boot.s
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel.o: kernel.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/virtio_gpu_driver.o: virtio_gpu_driver/virtio_gpu_driver.cpp virtio_gpu_driver/virtio_gpu_driver.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/pci_driver.o: pci_driver/pci_driver.c pci_driver/pci_driver.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/uart.o: uart_driver/uart.c uart_driver/uart.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/page_allocator.o: page_allocator/page_bump_allocator.c page_allocator/page_allocator.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel_allocator.o: kernel_allocator/kernel_allocator.c kernel_allocator/kernel_allocator.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/init_mmu.o: mmu/init_mmu.S
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/mmu.o: mmu/mmu.c mmu/mmu.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/terminal.o: terminal/terminal.cpp terminal/terminal.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/formating.o: std/formating.c std/formating.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel_logger.o: kernel_logs/kernel_logger.cpp kernel_logs/kernel_logger.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel.elf: linker.ld $(OBJ_FILES)
	$(CC) $(LDFLAGS) $(OBJ_FILES) -o $@

$(BUILD_DIR)/kernel.img: $(BUILD_DIR)/kernel.elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILD_DIR)
