[[ !  -z  $1  ]] && echo Specify a folder to save the backup $1 || exit
BACKUP_DIR=${PWD}/$1

. ../../global_exports.sh
echo $1/${KERNEL_VMLINUX}
${EXTRACT_VMLINUX} $1/${KERNEL_VMLINUX} > $1/vmlinux.img
 objdump -d  ${BACKUP_DIR}/vmlinux.img >   $1/vmlinux.asm_view
