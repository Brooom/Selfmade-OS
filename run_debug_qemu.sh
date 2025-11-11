#!/usr/bin/env bash
FILE="$TMPDIR/out-$(date +%Y%m%d-%H%M%S).txt"
qemu-system-aarch64 -M virt -cpu cortex-a53 -device virtio-gpu-pci -m 256M \
 -serial stdio -kernel build/kernel.elf -monitor unix:/tmp/qemu-monitor-socket,server,nowait -s -S 2>&1
| tee "$FILE"
echo "Saved to: $FILE

