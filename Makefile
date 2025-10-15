CC      = aarch64-elf-gcc
OBJCOPY = aarch64-elf-objcopy

CFLAGS  = -ffreestanding
LDFLAGS = -nostdlib -nostartfiles -T linker.ld

OBJS = boot.o kernel.o uart.o

all: kernel.img

boot.o: boot.S
	$(CC) $(CFLAGS) -c $< -o $@

kernel.o: kernel.c uart.h
	$(CC) $(CFLAGS) -c $< -o $@

uart.o: uart.c uart.h
	$(CC) $(CFLAGS) -c $< -o $@

kernel.elf: linker.ld $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

kernel.img: kernel.elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -f $(OBJS) kernel.elf kernel.img
