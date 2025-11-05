#!/usr/bin/env bash
FILE="$TMPDIR/out-$(date +%Y%m%d-%H%M%S).txt"
qemu-system-aarch64 -M virt -cpu cortex-a53 -device virtio-gpu-pci -m 256M \
 -serial stdio -kernel kernel.elf 2>&1 | tee "$FILE"
echo "Saved to: $FILE