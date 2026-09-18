#!/usr/bin/env bash
FILE="$TMPDIR/out-$(date +%Y%m%d-%H%M%S).txt"
qemu-system-aarch64 \
    -M virt,gic-version=3\
    -gic-version 3 \
    -cpu cortex-a53 \
    -device virtio-gpu-pci \
    -m 256M \
    -chardev stdio,mux=on,id=console0,logfile=$FILE \
    -serial chardev:console0 \
    -kernel build/kernel.elf \
    -monitor unix:/tmp/qemu-monitor-socket.sock,server,nowait -s -S
echo "Saved to: $FILE"

#To start the qemu monitor use "socat -,raw,echo=0 unix-connect:/tmp/qemu-monitor-socket.sock"

