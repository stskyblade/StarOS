cmake_minimum_required(VERSION 3.30)
project(StarOS
  VERSION 0.1
  LANGUAGES C CXX ASM)


# the name of the target operating system
set(CMAKE_SYSTEM_NAME Generic)

# which compilers to use for C and C++
set(TOOLCHAIN_PREFIX i686-elf)
set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}-as)
set(OBJCOPY ${TOOLCHAIN_PREFIX}-objcopy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-ffreestanding -Wall -Wno-write-strings -Wno-unused-variable -Wno-unused-parameter -Wextra -fno-exceptions -fno-rtti -nostdlib -g")

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(BOOTLOADER_KERNEL_SHARED_FILES
  ${SRC_DIR}/bootloader32.h
  ${SRC_DIR}/b32_print.cpp
  ${SRC_DIR}/b32_disk.cpp
  ${SRC_DIR}/b32_filesystem.cpp
  ${SRC_DIR}/b32_string.cpp
  ${SRC_DIR}/b32_utils.cpp
)

add_library(bootloadershared
  STATIC
  ${BOOTLOADER_KERNEL_SHARED_FILES}
)

add_executable(mbr.elf
  ${SRC_DIR}/mbr.S
  ${SRC_DIR}/bootloader.S
  ${SRC_DIR}/bootloader_32.cpp
)
target_link_options(mbr.elf PRIVATE -T${SRC_DIR}/mbr.ld)
target_link_libraries(mbr.elf PUBLIC bootloadershared)

set(KERNEL_FILE staros_kernel.elf)
add_executable(${KERNEL_FILE}
  ${SRC_DIR}/interrupt_handlers.S
  ${SRC_DIR}/kernel.cpp
)
target_link_options(${KERNEL_FILE} PRIVATE -T${SRC_DIR}/kernel.ld)
target_link_libraries(${KERNEL_FILE} PUBLIC bootloadershared)

set(DISK_IMG_FILENAME disk.img)
add_custom_target(
  disk.img
  ALL
  DEPENDS mbr.elf ${PROJECT_BINARY_DIR}/fs.img
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  BYPRODUCTS mbr.bin ${DISK_IMG_FILENAME}
  COMMAND ${OBJCOPY} -O binary --only-section=.text mbr.elf mbr.bin
  # # make a empty 200MB disk image
  # COMMAND dd status=none if=/dev/zero of=${DISK_IMG_FILENAME} bs=512 count=409600
  COMMAND cp fs.img ${DISK_IMG_FILENAME}
  # copy mbr code to the first sector of disk image
  COMMAND dd status=none if=./mbr.bin of=${DISK_IMG_FILENAME} conv=notrunc
  # copy text to the 65th sector of disk image
  COMMAND dd status=none if=${PROJECT_SOURCE_DIR}/root_dir/hello.txt of=${DISK_IMG_FILENAME} conv=notrunc seek=64
  # copy text to the 66th sector of disk image
  COMMAND dd status=none if=${PROJECT_SOURCE_DIR}/root_dir/world.txt of=${DISK_IMG_FILENAME} conv=notrunc seek=65
)

add_custom_target(
  qemu
  DEPENDS disk.img
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMAND qemu-system-i386 -m 5G -serial stdio ${DISK_IMG_FILENAME}
)

add_custom_target(
  debug
  DEPENDS disk.img
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMAND qemu-system-i386 -m 5G -serial stdio ${DISK_IMG_FILENAME} -s -S
)

# make a disk image
# partition 200MB
# have two text file
set(USB_DEVICE /dev/sdb)
set(MOUNT_POINT /mnt/fat32_device)
set(FS_IMG /tmp/fs.img)
set(LOOP_DEVICE /dev/loop0)
add_custom_command(
  OUTPUT ${PROJECT_BINARY_DIR}/fs.img
  DEPENDS ${KERNEL_FILE}
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  # COMMAND if [ ! -b ${USB_DEVICE} ]; then echo "File not found: ${USB_DEVICE}" \; exit 1 \; fi
  # COMMAND echo "Found /dev/sdb"
  # erase all data, first 250MB
  # COMMAND echo "Erase all data in first 250MB"
  COMMAND sudo dd if=/dev/zero of=${FS_IMG} count=512000
  COMMAND sudo losetup ${LOOP_DEVICE} ${FS_IMG}
  COMMAND echo "Partition..."
  COMMAND sudo sfdisk -q ${LOOP_DEVICE} < ${PROJECT_SOURCE_DIR}/partition_layout.dump
  COMMAND sudo partprobe ${LOOP_DEVICE}
  COMMAND sudo mkfs.fat -F 32 ${LOOP_DEVICE}p1
  COMMAND sudo mkdir -p ${MOUNT_POINT}
  COMMAND sudo mount ${LOOP_DEVICE}p1 ${MOUNT_POINT}
  COMMAND echo "Copy files..."
  COMMAND sudo cp ${PROJECT_BINARY_DIR}/${KERNEL_FILE} ${MOUNT_POINT}
  COMMAND sudo cp ${PROJECT_SOURCE_DIR}/root_dir/hello.txt ${MOUNT_POINT}
  COMMAND sudo cp ${PROJECT_SOURCE_DIR}/root_dir/world.txt ${MOUNT_POINT}
  COMMAND sudo cp ${PROJECT_SOURCE_DIR}/root_dir/large_hello.txt ${MOUNT_POINT}
  COMMAND sudo umount ${LOOP_DEVICE}p1
  COMMAND sudo rm -rf ${MOUNT_POINT}
  COMMAND sudo losetup -d ${LOOP_DEVICE}
  # directly manipulate image on project directory is extreamly slow
  COMMAND sudo cp /tmp/fs.img ${PROJECT_BINARY_DIR}/fs.img
  # COMMAND sudo dd status=none if=${USB_DEVICE} of=${PROJECT_BINARY_DIR}/fs.img count=512000
  COMMAND echo "Done."
)

# # only burn first 2048 sectors to USB drive
# add_custom_target(
#   burn
#   DEPENDS disk.img
#   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
#   COMMAND echo "Write to USB driver..."
#   COMMAND if [ ! -b /dev/sdb ]; then echo "File not found: /dev/sdb" \; exit 1 \; fi
#   COMMAND sudo dd status=none if=disk.img of=/dev/sdb bs=512 count=2048
#   COMMAND sync
# )


add_custom_target(
  burn
  DEPENDS disk.img
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMAND echo "Write to USB driver..."
  COMMAND if [ ! -b /dev/sdb ]; then echo "File not found: /dev/sdb" \; exit 1 \; fi
  COMMAND sudo dd status=none if=disk.img of=/dev/sdb bs=512 count=512000
  COMMAND sync
)

add_custom_target(
  dump
  COMMAND sudo dd status=none if=${USB_DEVICE} of=/tmp/sdb_250M.img count=512000
  COMMAND hexdump -C /tmp/sdb_250M.img
)

add_custom_command(
  OUTPUT ${SRC_DIR}/interrupt_handlers.S
  COMMAND ./codegen_interrupt_handlers.sh > ./interrupt_handlers.S
  DEPENDS ${SRC_DIR}/codegen_interrupt_handlers.sh
  WORKING_DIRECTORY ${SRC_DIR}
)