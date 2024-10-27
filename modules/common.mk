ifeq ($(__BUILD_ARCH), amd64)
CCSED='s/\-fconserve-stack//g; s/\-fno-stack-clash-protection//g; s/\-mindirect-branch-register//g; s/\-mindirect-branch=thunk-extern//g; s/\-mskip-rax-setup//g; s/\-mpreferred-stack-boundary=3//g; s/\-mno-fp-ret-in-387//g; s/\-mrecord-mcount//g '
__BUILD_ARCH=amd64
else 
CCSED='s/\-fconserve-stack//g; s/\-fno-stack-clash-protection//g; s/\-mindirect-branch-register//g; s/\-mindirect-branch=thunk-extern//g; s/\-mskip-rax-setup//g; s/\-mpreferred-stack-boundary=3//g; s/\-mno-fp-ret-in-387//g; s/\-femit-struct-debug-baseonly//g; s/\-mstack-protector-guard=sysreg//g; s/\-mstack-protector-guard-reg=sp_el0//g; s/\-mstack-protector-guard-offset=1152//g'
__BUILD_ARCH=aarch64
endif

obj-m += module.o

all: module.ko

module.ko: module.c
	make -C /sources/linux M=$(PWD) modules

clean:
	make -C /sources/linux M=$(PWD) clean
	rm -rf .clangd .cache *.gz compile_commands.json compile_commands.json.in bzImage .config

compile_commands.json:
	make -C /sources/linux M=$(PWD) $@

setup-cc: 
	rm -f compile_commands.json
	make compile_commands.json
	cp compile_commands.json compile_commands.json.in
	cat compile_commands.json.in | sed $(CCSED) > compile_commands.json

# https://github.com/Mange/rtl8192eu-linux-driver/issues/205#issuecomment-736366347
prepare: 
	cp ../aux/module.lds.S-$(__BUILD_ARCH) /sources/linux/scripts/module.lds

copy-to-fs: module.ko
	mkdir -p /staging/initramfs/fs/modules
	cp module.ko /staging/initramfs/fs/modules/$(shell basename `pwd`).ko

test: 
	make prepare
	make module.ko 
	make copy-to-fs 
	/sources/mini-bootstrap.sh

rebuild: copy-to-fs
	make -C .. rebuild



