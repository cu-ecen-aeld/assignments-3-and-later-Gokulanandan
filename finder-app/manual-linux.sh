#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-
CURRENT_DIR=$(pwd)

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

if [ $? -ne 0 ]; then
	echo "output directory could not be created"
	exit 1;
fi


cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    echo "Compiling targets... "

    # TODO: Add your kernel build steps here
    # cleanup
    make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- mrproper

    #make defconfig
    make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- defconfig

    #make vmlinux
    make -j4 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- all

    #make modules
    #make -j4 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu-modules

    #make device tree
    make -j4 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- dtbs
fi

echo "Adding the Image in outdir"

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories

mkdir -p ${OUTDIR}/rootfs ${OUTDIR}/rootfs/home
if [ $? -ne 0 ]; then
	echo "rootfs directory could not be created"
	exit 1;
fi

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
else
    cd busybox
fi

# TODO: Make and install busybox

echo "Compiling and installing busybox"
make distclean
make defconfig
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install 

echo "Library dependencies"
CMD=$(${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter" | awk '{print $NF}')
CMD1=$(${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library" | awk '{print $NF}')

echo "cmd is $CMD"
echo "cmd1 is $CMD"

# TODO: Add library dependencies to rootfs
cp $CMD $OUTDIR/rootfs/lib
cp $CMD1 $OUTDIR/rootfs/lib64

# TODO: Make device nodes

cd "$OUTDIR/rootfs"
sudo mknod  -m 666 dev/null 1 2
sudo mknod  -m 666 dev/console 5 1

# TODO: Clean and build the writer utility

echo "Cross compiling write utility"
cd $CURRENT_DIR
make clean
make CROSS_COMPILE=aarch64-none-linux-gnu-

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp finder.sh conf/assignment.txt conf/username.txt autorun-qemu.sh $OUTDIR/rootfs/home

# TODO: Chown the root directory
sudo chown user:group ${OUTDIR}/rootfs

# TODO: Create initramfs.cpio.gz

cd "$OUTDIR/rootfs"

find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
gzip -f initramfs.cpio
