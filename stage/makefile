ifeq ($(__BUILD_ARCH), amd64)
QEMUCMD=qemu-system-x86_64
QEMUMAC=-smp 4 -nographic
QEMUAPP="console=ttyS0 init=/init" 
__BUILD_ARCH=amd64
else 
QEMUCMD=qemu-system-aarch64
QEMUMAC=-smp 1 -machine virt -cpu cortex-a57 -nographic
QEMUAPP="console=ttyAMA0 init=/init" 
__BUILD_ARCH=aarch64
endif

start: 
	$(QEMUCMD) $(QEMUMAC) \
		-kernel bzImage-$(__BUILD_ARCH) \
		-initrd initramfs-busybox-${__BUILD_ARCH}.cpio.gz \
		-append $(QEMUAPP)






