#!/usr/bin/env bash
FILE="$TMPDIR/out-$(date +%Y%m%d-%H%M%S).txt"
qemu-system-aarch64 \
    -M virt,gic-version=3\ #-M virt,gic-version=3,dumpdtb=build/virt.dtb  generates device tree blob
    -cpu cortex-a53 \
    -device virtio-gpu-pci \
    -m 256M \
    -chardev stdio,mux=on,id=console0,logfile=$FILE \
    -serial chardev:console0 \
    -kernel build/kernel.elf \
    -monitor unix:/tmp/qemu-monitor-socket,server,nowait
echo "Saved to: $FILE"
