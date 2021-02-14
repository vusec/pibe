#!/bin/bash

. global_exports.sh


echo "Kernel version: ${KERNEL_VERSION}"
echo "Vmlinux name: ${KERNEL_VMLINUX}"
echo "Config name: ${KERNEL_CONFIG}"
echo "Initrd name: ${KERNEL_INITRD}"
echo "System map name: ${KERNEL_MAP}"
[[ !  -z  $1  ]] && QEMU_DIR=$WD/playground/${WORKLOAD_DIR}/${1}
echo "Qemu dir: ${QEMU_DIR}"
[ ! -d ${QEMU_DIR} ] && mkdir -p ${QEMU_DIR}

rm -f ${QEMU_DIR}/${KERNEL_VMLINUX}  ${QEMU_DIR}/${KERNEL_CONFIG} ${QEMU_DIR}/${KERNEL_INITRD} ${QEMU_DIR}/${KERNEL_MAP}
rm -f ${QEMU_DIR}/${KERNEL_VMLINUX_OBJECT}  ${QEMU_DIR}/${KERNEL_VMLINUX_DUMP}

cp ${KERNEL_DIR}/${KERNEL_VMLINUX} ${QEMU_DIR}/${KERNEL_VMLINUX}
cp ${KERNEL_DIR}/${KERNEL_CONFIG} ${QEMU_DIR}/${KERNEL_CONFIG}
cp ${KERNEL_DIR}/${KERNEL_INITRD} ${QEMU_DIR}/${KERNEL_INITRD}
cp ${KERNEL_DIR}/${KERNEL_MAP}  ${QEMU_DIR}/${KERNEL_MAP}

./disassemble_kernel.sh

