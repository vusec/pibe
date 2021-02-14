#!/bin/bash
SOURCE="${BASH_SOURCE[0]}"
export WD=$(dirname $SOURCE)
export KERNEL_VERSION=5.1.0
export BASE_EXTRA_VERSION=1
export HEADERS_VERSION=4.15.0-91-generic
export KERNEL_DIR=/boot
export KERNEL_VMLINUX=vmlinuz-$KERNEL_VERSION
export KERNEL_CONFIG=config-$KERNEL_VERSION
export KERNEL_INITRD=initrd.img-$KERNEL_VERSION
export KERNEL_MAP=System.map-$KERNEL_VERSION
export WORKLOAD_DIR=metadata
export KERNEL_VMLINUX_OBJECT=vmlinux
export KERNEL_VMLINUX_DUMP=vmlinux_dump
export QEMU_DIR=$WD/playground/${WORKLOAD_DIR}/qemudir
export KERNEL_PREFIX=$WD/kernel/mainline
export PASS_INCLUDE_DIR=$WD/llvm-includes
export PASS_EXCLUDE_HEADER=exclude_functions.h
export LIB_DIR=/lib/modules/${KERNEL_VERSION}
export DEFAULT_KERNEL_SOURCE_TREE_PREFIX=$WD/kernel
export USED_KERNEL_SOURCE=backport
export EXTRACT_VMLINUX=${DEFAULT_KERNEL_SOURCE_TREE_PREFIX}/${USED_KERNEL_SOURCE}/scripts/extract-vmlinux
export BENCHMARK_DIR=performance
[ -z "$RUNS" ] && export RUNS=5

# The following environment variables are used by make_rootfs.sh to make a rootfs for qemu execution
export ROOTFS_DIR=${QEMU_DIR}/rootfs
export ROOTFS_IMG=rootfs.img
export MODULE_NAME=shadow-module.ko
export MODULE_PATH=$WD/modules/release
export MKNOD_SCRIPT=mknod.sh
export IOCTL_SCRIPT=try_ioctl
