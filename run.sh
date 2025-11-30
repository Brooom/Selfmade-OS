#!/usr/bin/env bash
FILE="$TMPDIR/out-$(date +%Y%m%d-%H%M%S).txt"
qemu-system-aarch64 \
    -M virt \
    -cpu cortex-a53 \
    -device virtio-gpu-pci \
    -m 256M \
    -chardev file,id=logfile,path="$FILE" \
    -chardev stdio,id=console0 \
    -chardev mux,id=mux0,chardev=console0,chardev=logfile \
    -serial chardev:console0 \
    -kernel build/kernel.elf
echo "Saved to: $FILE