> A simply reproducible Linux kernel development environment. It creates a sandbox container where you can checkout, compile and run a modern linux kernel.

## Setting up the container
These instructions will get you up and running with a working kernel that can be run with qemu. This is done by setting up a container that will download all the dependencies to compile version 5.11 of the Linux kernel and build a busybox-based initramfs. You can decide whether to have a kernel built for `amd64` or `aarch64` (experimental) by using the `T` environment variable while building the container:

```
T=amd64 make build-container
```

## Generating and running your first kernel image
To compile the kernel image (and a boot disc called `initramfs`):
```
T=amd64 make build-sys
```

All the modules in the `modules` folder will be compiled and copied into the boot disc so that you can play with them. The actual kernel image and the initramfs disc will be created in `stage`

```
$ ls -l stage
.rw-r--r-- zaccaria staff 444 B  Thu Apr 18 10:28:32 2024 -- makefile
.rw-r--r-- zaccaria staff 1.2 MB Thu May  9 16:55:21 2024 .I initramfs-busybox-amd64.cpio.gz
.rw-r--r-- zaccaria staff  32 MB Thu May  9 16:55:22 2024 -- bzImage-amd64
```

To run such kernel with the created ramdisk run 

```
$ stage/start-qemu.sh --arch amd64   
```


## References

- https://www.collabora.com/news-and-blog/blog/2017/01/16/setting-up-qemu-kvm-for-kernel-development/
- https://github.com/gurugio/book_linuxkernel_blockdrv/blob/master/environment.md
- https://github.com/cirosantilli/linux-kernel-module-cheat#u-boot
