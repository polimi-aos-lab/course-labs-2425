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

mkdir -p /staging/initramfs/fs

cd /sources/linux
# make ${df_file} // now we use a .config stored in the repo
make -j4 ${im_file}

# build busybox
cd /sources/busybox-1.32.1
make defconfig
LDFLAGS="--static" make -j4 install
cp ${im_file_path} /staging/bzImage-$__BUILD_ARCH

mkdir -p /staging/initramfs/fs

cd /staging/initramfs/fs
mkdir -pv bin sbin etc proc sys usr/bin usr/sbin
cp -av /sources/busybox-1.32.1/_install/* .
cp /sources/init .

if [ ! -f "/staging/initramfs/fs/bin/perf" ]; then
        cd /sources/linux/tools/perf
        LDFLAGS=-static NO_LIBPYTHON=1 make
        cp perf /staging/initramfs/fs/bin
fi

find . -print0 | cpio --null -ov --format=newc | gzip -9 > /staging/initramfs-busybox-${__BUILD_ARCH}.cpio.gz

if [ -d "/repo/stage" ]; then
        cp /staging/initramfs-busybox-${__BUILD_ARCH}.cpio.gz /repo/stage
        cp /staging/bzImage-$__BUILD_ARCH /repo/stage
fi
