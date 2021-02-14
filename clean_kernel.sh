#!/bin/bash
. global_exports.sh

sudo rm -r -f $LIB_DIR
sudo rm -f ${KERNEL_DIR}/${KERNEL_VMLINUX}
sudo rm -f ${KERNEL_DIR}/${KERNEL_CONFIG}
sudo rm -f ${KERNEL_DIR}/${KERNEL_INITRD}
sudo rm -f ${KERNEL_DIR}/${KERNEL_MAP}
sudo update-grub2
