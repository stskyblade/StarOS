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
add_executable(mbr.elf
  ${SRC_DIR}/mbr.S
  ${SRC_DIR}/bootloader.S
  ${SRC_DIR}/bootloader_32.cpp
  ${SRC_DIR}/b32_print.cpp
  ${SRC_DIR}/b32_disk.cpp
)
target_link_options(mbr.elf PRIVATE -T${SRC_DIR}/mbr.ld)

# add_custom_command(
#   TARGET mbr.elf
#   POST_BUILD
#   COMMAND ${OBJCOPY} -O binary --only-section=.text mbr.elf mbr.bin
#   COMMAND dd status=none if=/dev/zero of=./mbr.img bs=512 count=409600
#   COMMAND dd status=none if=./mbr.bin of=./mbr.img conv=notrunc
#   COMMAND dd status=none if=${PROJECT_SOURCE_DIR}/root_dir/hello.txt of=./mbr.img conv=notrunc seek=64
#   COMMAND dd status=none if=${PROJECT_SOURCE_DIR}/root_dir/world.txt of=./mbr.img conv=notrunc seek=65
#   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
# )

set(DISK_IMG_FILENAME disk.img)
add_custom_target(
  disk.img
  ALL
  DEPENDS mbr.elf fs
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
  COMMAND qemu-system-i386 -serial stdio ${DISK_IMG_FILENAME}
)

add_custom_target(
  debug
  DEPENDS disk.img
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMAND qemu-system-i386 -serial stdio ${DISK_IMG_FILENAME} -s -S
)

# make a disk image
# partition 200MB
# have two text file
set(USB_DEVICE /dev/sdb)
set(MOUNT_POINT /mnt/fat32_device)
add_custom_target(
  fs
  BYPRODUCTS fs.img
  COMMAND if [ ! -b ${USB_DEVICE} ]; then echo "File not found: ${USB_DEVICE}" \; exit 1 \; fi
  COMMAND echo "Found /dev/sdb"
  # erase all data, first 250MB
  COMMAND echo "Erase all data in first 250MB"
  COMMAND sudo dd if=/dev/zero of=${USB_DEVICE} count=512000
  COMMAND echo "Partition..."
  COMMAND sudo sfdisk ${USB_DEVICE} < ${PROJECT_SOURCE_DIR}/partition_layout.dump
  COMMAND sudo mkfs.fat -F 32 ${USB_DEVICE}1
  COMMAND sudo mkdir -p ${MOUNT_POINT}
  COMMAND sudo mount ${USB_DEVICE}1 ${MOUNT_POINT}
  COMMAND echo "Copy files..."
  COMMAND sudo cp ${PROJECT_SOURCE_DIR}/root_dir/hello.txt ${MOUNT_POINT}
  COMMAND sudo cp ${PROJECT_SOURCE_DIR}/root_dir/world.txt ${MOUNT_POINT}
  COMMAND sudo cp ${PROJECT_SOURCE_DIR}/root_dir/large_hello.txt ${MOUNT_POINT}
  COMMAND sudo umount ${USB_DEVICE}1
  COMMAND sudo rm -rf ${MOUNT_POINT}
  COMMAND sudo dd status=none if=${USB_DEVICE} of=fs.img count=512000
  COMMAND echo "Done."
)

add_custom_target(
  dump
  COMMAND sudo dd status=none if=${USB_DEVICE} of=/tmp/sdb_250M.img count=512000
  COMMAND hexdump -C /tmp/sdb_250M.img
)