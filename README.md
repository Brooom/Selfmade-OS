---
marp: false
---

# Selfmade OS

Selfmade OS is a tiny 64-bit bare-metal kernel. It currently targets QEMU's `virt` machine with a VirtIO-GPU framebuffer while aiming to grow into a Raspberry Pi oriented OS.

## What’s here
- Boot shim (`boot.s`) that jumps into the C++ kernel (`kernel.cpp`) via the custom linker script.
- Minimal runtime support: bump allocator (`allocator.c`), lightweight formatting helpers (`std/`), and a kernel logger/terminal combo.
- Basic drivers for UART serial output, PCI probing, and a VirtIO-GPU 2D renderer that paints the on-screen terminal.

## Build
Prerequisites: `aarch64-elf-gcc/g++`, `make`, and `qemu-system-aarch64`.

```sh
make
```

The build drops `build/kernel.elf` (ELF for QEMU) and `build/kernel.img` (raw image).

## Run under QEMU
```sh
./run.sh
```
The helper script boots `qemu-system-aarch64 -M virt …` and mirrors the serial log to both the terminal and a timestamped file in `$TMPDIR`.

## Run under QEMU and debug
In terminal 1:
```sh
./run_debug_quemu.sh
```
Starts quemu for debugging.

In terminal 2:
```sh
./run_debug_gdb.sh
```
Starts gdb for debugging.

## Toward Raspberry Pi
- Swap the current VirtIO-GPU and PCI paths with Raspberry Pi MMIO peripherals (framebuffer/UART/interrupts).
- Flesh out memory management and higher-level services once the hardware bring-up is stable.
