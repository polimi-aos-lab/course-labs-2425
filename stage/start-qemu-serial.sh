#!/bin/bash
# Start QEMU with the first serial (ttyS0) mapped to stdio, and the second one mapped to port 6000
#
# Note: for some reason, qemu must be started in the container for this to work!
# BUG: with -serial stdio it is not possible to stop QEMU with CTRL-A x

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
qemu-system-x86_64 -smp 4 -kernel $DIR/bzImage-amd64 -initrd $DIR/initramfs-busybox-amd64.cpio.gz -append "console=ttyS0 init=/init" -vnc :0 -k en-us -serial stdio -serial tcp::6000,server

