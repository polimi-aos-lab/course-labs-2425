#!/bin/sh
#

if [ "$__BUILD_ARCH" = "amd64" ]; then 
        # df_file=x86_64_defconfig 
        im_file=bzImage
        im_file_path=/sources/linux/arch/x86_64/boot/bzImage
elif [ "$__BUILD_ARCH" = "aarch64" ]; then 
        # df_file=defconfig 
        im_file=Image
        im_file_path=/sources/linux/arch/arm64/boot/Image
else 
	echo "Architecture $__BUILD_ARCH not supported"
	exit 
fi

cd /staging/initramfs/fs
cp /sources/init .

find . -print0 | cpio --null -ov --format=newc | gzip -9 > /staging/initramfs-busybox-${__BUILD_ARCH}.cpio.gz

if [ -d "/repo/stage" ]; then
        cp /staging/initramfs-busybox-${__BUILD_ARCH}.cpio.gz /repo/stage
        cp /staging/bzImage-$__BUILD_ARCH /repo/stage
fi
