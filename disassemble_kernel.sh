#!/bin/bash
#. global_exports.sh
sudo $EXTRACT_VMLINUX ${QEMU_DIR}/${KERNEL_VMLINUX} >  ${QEMU_DIR}/${KERNEL_VMLINUX_OBJECT}

objdump -d --insn-width=15  ${QEMU_DIR}/${KERNEL_VMLINUX_OBJECT} > ${QEMU_DIR}/${KERNEL_VMLINUX_DUMP}

cat ${QEMU_DIR}/${KERNEL_VMLINUX_DUMP} | grep call > ${QEMU_DIR}/call_stats


