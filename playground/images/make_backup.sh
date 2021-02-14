#!/bin/bash
[[ !  -z  $1  ]] && echo Specify a folder to save the backup $1 || exit

BACKUP_DIR=${PWD}/$1

. ../../global_exports.sh
echo "Kernel version: ${KERNEL_VERSION}"
echo "Vmlinux name: ${KERNEL_VMLINUX}"
echo "Config name: ${KERNEL_CONFIG}"
echo "Initrd name: ${KERNEL_INITRD}"
echo "System map name: ${KERNEL_MAP}"
echo "Backup dir: ${BACKUP_DIR}"

if [ $2 == clean ]; then
    rm -f ${BACKUP_DIR}/vmlinux.img
    rm -f ${BACKUP_DIR}/vmlinux.asm_view
    exit
fi


rm -f -r ${BACKUP_DIR}
mkdir ${BACKUP_DIR}
#rm -f ${QEMU_DIR}/${KERNEL_VMLINUX}  ${QEMU_DIR}/${KERNEL_CONFIG} ${QEMU_DIR}/${KERNEL_INITRD} ${QEMU_DIR}/${KERNEL_MAP}
#rm -f ${QEMU_DIR}/${KERNEL_VMLINUX_OBJECT}  ${QEMU_DIR}/${KERNEL_VMLINUX_DUMP}

cp ${KERNEL_DIR}/${KERNEL_VMLINUX} ${BACKUP_DIR}/${KERNEL_VMLINUX}
cp ${KERNEL_DIR}/${KERNEL_CONFIG} ${BACKUP_DIR}/${KERNEL_CONFIG}
cp ${KERNEL_DIR}/${KERNEL_INITRD} ${BACKUP_DIR}/${KERNEL_INITRD}
cp ${KERNEL_DIR}/${KERNEL_MAP}  ${BACKUP_DIR}/${KERNEL_MAP}

cp -r ${LIB_DIR} ${BACKUP_DIR}/${KERNEL_VERSION}

if [ $2 == asm ]; then
    ${EXTRACT_VMLINUX} ${BACKUP_DIR}/${KERNEL_VMLINUX} > ${BACKUP_DIR}/vmlinux.img
    objdump -d  ${BACKUP_DIR}/vmlinux.img >  ${BACKUP_DIR}/vmlinux.asm_view
fi







