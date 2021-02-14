

. ../../global_exports.sh

echo ${1}/${KERNEL_VMLINUX}  ${QEMU_DIR}/${ROOTFS_IMG}
sudo qemu-system-x86_64 -cpu Nehalem -gdb tcp::1234 -kernel ${1}/${KERNEL_VMLINUX} -initrd ${1}/initrd.img-5.1.0  -append "root=/dev/ram rdinit=/sbin/init loglevel=7 console=tty0 console=ttyS0 rw " -serial stdio -m 2G
