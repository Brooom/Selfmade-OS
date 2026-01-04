#!/usr/bin/env bash
FILE="$TMPDIR/out-$(date +%Y%m%d-%H%M%S).txt"
qemu-system-aarch64 \
    -M virt \
    -cpu cortex-a53 \
    -device virtio-gpu-pci \
    -m 256M \
    -chardev stdio,mux=on,id=console0,logfile=$FILE \
    -serial chardev:console0 \
    -kernel build/kernel.elf \
    -monitor unix:/tmp/qemu-monitor-socket,server,nowait -s -S
echo "Saved to: $FILE