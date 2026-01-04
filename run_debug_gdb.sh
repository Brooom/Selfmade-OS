#!/usr/bin/env bash
aarch64-elf-gdb -ex "file build/kernel.elf" -ex "target remote :1234"