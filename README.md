# Selfmade OS

Selfmade OS is a personal 64-bit bare-metal kernel project. It currently targets QEMU's AArch64 `virt` machine with a Cortex-A53 CPU and VirtIO-GPU. The long-term goal is to port the kernel to Raspberry Pi hardware. The project is experimental and under active development.

## Milestones

1. Develop the core kernel on QEMU's `virt` board.
   - Implement process scheduling.
   - Replace the bump allocator with a buddy allocator.
2. Port the kernel to Raspberry Pi hardware.
3. Continue expanding the kernel.

## Project status

- [x] AArch64 boot
- [x] Exception vector table
- [x] MMU and page-table mapping
- [x] Basic kernel allocation
- [x] PL011 serial output
- [x] PCI device discovery
- [x] VirtIO-GPU framebuffer
- [ ] Interrupt-controller support
- [ ] Timer interrupts
- [ ] Process scheduling
- [ ] Buddy allocator
- [ ] Raspberry Pi platform support

## Build
Prerequisites: `aarch64-elf-gcc`, `aarch64-elf-g++`, `make`, and `qemu-system-aarch64 10.2.0`.

```sh
make
```

For a clean rebuild, remove all previous build output first:

```sh
make clean && make
```

The build produces `build/kernel.elf` (ELF for QEMU) and `build/kernel.img` (raw image).

## Run under QEMU
```sh
./run.sh
```
The helper script boots `qemu-system-aarch64 -M virt …` and mirrors the serial log to both the terminal and a timestamped file in `$TMPDIR`.

## Run under QEMU and debug
Start QEMU paused with its GDB server enabled:
```sh
./run_debug_qemu.sh
```
In another terminal, connect GDB:

In terminal 2:
```sh
./run_debug_gdb.sh
```
Starts gdb for debugging.


## File overview

Files marked as **not built** are present in the repository but are not listed in `Makefile`'s object-file list.

| File | Purpose |
| --- | --- |
| `.gdbinit` | Defines GDB helper commands for rebuilding, reloading, and resetting the kernel during a QEMU debugging session. |
| `Makefile` | Cross-compiles the AArch64 assembly, C, and C++ sources. Links `build/kernel.elf` and creates the raw `build/kernel.img`. |
| `boot.s` | Contains the AArch64 exception vector table and `_start`, installs the vector table, initializes the stack, and calls `kernel_main`. |
| `kernel.cpp` | Implements `kernel_main`, the current kernel entry point, and initializes memory allocation, PCI, VirtIO-GPU output, and the terminal while using UART logging and demo rendering. |
| `linker.ld` | Defines the kernel memory layout, entry point, initial MMU pages, code and data sections, exception vectors, stack, and heap boundaries. |
| `exceptions/exceptions.c` | Implements handlers for the 16 AArch64 exception-vector entries and logs exception information. |
| `kernel_allocator/kernel_allocator.c` | Implements page-granularity kernel allocation by obtaining physical pages, mapping them through the MMU, and clearing the allocated memory. |
| `kernel_allocator/kernel_allocator.h` | Declares the kernel allocator initialization, allocation, and free interfaces. |
| `kernel_logs/kernel_logger.cpp` | Implements formatted kernel logging to UART and, after initialization, to the graphical terminal. It also provides the C-callable logging wrappers. |
| `kernel_logs/kernel_logger.hpp` | Declares the C++ `kernel_logger` singleton and its terminal integration. |
| `kernel_logs/kernel_logger_c_api.h` | Exposes the logger to C source files through `extern "C"` functions. |
| `mmu/init_mmu.S` | Programs the AArch64 `MAIR_EL1` and `TCR_EL1` registers with the initial memory-attribute and translation settings. |
| `mmu/mmu.c` | Builds and manages AArch64 translation tables, maps allocated pages, and registers MMIO regions as device memory. |
| `mmu/mmu.h` | Defines page-table constants, memory attributes, address-range constants, and the public MMU interface. |
| `mmu/mmu.md` | Documents the current on-demand mapping design, its relationship to the allocators, and known optimization work. |
| `page_allocator/page_allocator.h` | Declares the page allocator interface used by the kernel allocator and MMU. |
| `page_allocator/page_bump_allocator.c` | Implements the active simple bump page allocator and contiguous page allocation. Allocated pages cannot currently be freed. |
| `page_allocator/page_free_linked_list_allocator.c` | Contains an unfinished linked-list allocator experiment. **Not currently built.** |
| `pci_driver/pci_driver.c` | Implements PCI ECAM access, device discovery, VirtIO capability traversal, BAR sizing, and BAR memory allocation for QEMU `virt`. |
| `pci_driver/pci_driver.h` | Declares PCI data structures and functions and defines the QEMU `virt` ECAM and PCI address ranges. |
| `run.sh` | Starts the kernel normally on QEMU's AArch64 `virt` machine with a Cortex-A53 CPU, VirtIO-GPU, serial logging, and a monitor socket. |
| `run_debug_gdb.sh` | Starts AArch64 GDB, loads the kernel symbols, and connects to QEMU's GDB server on port 1234. |
| `run_debug_qemu.sh` | Starts the same QEMU machine paused with its GDB server enabled, ready for `run_debug_gdb.sh`. |
| `std/args.h` | Provides freestanding aliases for the compiler's built-in variadic-argument operations. |
| `std/formating.c` | Implements the kernel's small `printf`-style formatter, including strings, characters, unsigned integers, hexadecimal values, pointers, and literal percent signs. |
| `std/formating.h` | Declares the string formatter and its integer-conversion helpers. |
| `terminal/terminal.cpp` | Implements a graphical scrolling terminal backed by a circular text buffer and rendered through the VirtIO-GPU driver. |
| `terminal/terminal.hpp` | Declares the terminal class, text limits, circular-buffer state, and GPU dependency. |
| `uart_driver/uart.c` | Implements formatted serial output through the PL011 UART at QEMU `virt` address `0x09000000` and registers its MMIO page. |
| `uart_driver/uart.h` | Declares UART initialization and formatted transmission functions. |
| `virtio_gpu_driver/letters/font8x8_basic.h` | Contains the public-domain 8-by-8 bitmap glyph data used to render Basic Latin characters. |
| `virtio_gpu_driver/letters/notice.md` | Records the bitmap font's source, revision, and public-domain license. |
| `virtio_gpu_driver/virtio_gpu_driver.cpp` | Implements VirtIO-GPU PCI initialization, virtqueue commands, framebuffer setup, scanout updates, and basic pixel, shape, character, and text drawing. |
| `virtio_gpu_driver/virtio_gpu_driver.hpp` | Declares VirtIO queue and graphics data structures and the public `virtio_gpu_driver` class. |
