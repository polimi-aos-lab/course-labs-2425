#!/bin/bash

show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  --arch <arch>            Specify architecture ('amd64' or 'aarch64')"
    echo "  --smp <n>                Number of processors (default 1)"
    echo "  --with-gui               Open QEMU gui"
    echo "  --dry                    Dry run"
    echo "  --help                   Display this help and exit"
}

QEMUMAC=""
QEMUCMD=""
QEMUAPP=""
BUILD_ARCH=""
DRY=0
SMP=1
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"


for arg in "$@"; do
  case $arg in
    --arch)
      ARCH=$2
      shift # Remove argument name from processing
      shift # Remove argument value from processing
      ;;
    --smp)
      SMP=$2
      shift # Remove argument name from processing
      shift # Remove argument value from processing
      ;;
    --with-gui)
      GRAPHIC_MODE=1
      shift # Remove flag from processing
      ;;
    --dbg)
      DEBUG_MODE=1
      shift # Remove flag from processing
      ;;
    --dry)
      DRY=1
      shift # Remove flag from processing
      ;;
    --help)
      show_help
      exit 0
      ;;
    *) 
      shift
      ;;
  esac
done

if [ "$ARCH" == "amd64" ]; then
    QEMUCMD="qemu-system-x86_64"
    QEMUMAC="-smp $SMP"
    if [ -z "$GRAPHIC_MODE" ]; then
       QEMUMAC+=" -nographic"
       QEMUAPP="console=ttyS0 init=/init nokaslr"
    else 
       QEMUAPP="init=/init nokaslr"
    fi
    BUILD_ARCH="amd64"
elif [ "$ARCH" == "aarch64" ]; then
    QEMUCMD="qemu-system-aarch64"
    QEMUMAC="-smp $SMP -machine virt -cpu cortex-a57"
    if [ -z "$GRAPHIC_MODE" ]; then
       QEMUMAC+=" -nographic"
       QEMUAPP="console=ttyAMA0 init=/init nokaslr"
    else 
       QEMUAPP="init=/init nokaslr"
    fi
    BUILD_ARCH="aarch64"
else
    echo "Error: Architecture not specified or is invalid."
    exit 1
fi

if [ "$DEBUG_MODE" == "1" ]; then 
	QEMUMAC+=" -s -S"
fi

CMD="$QEMUCMD $QEMUMAC -kernel $DIR/bzImage-$BUILD_ARCH -initrd $DIR/initramfs-busybox-$BUILD_ARCH.cpio.gz -append \"$QEMUAPP\""

echo -e "${CMD}" 

if [ "$DRY" == "0" ]; then 
        eval $CMD
fi
