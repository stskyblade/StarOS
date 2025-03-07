cmake_minimum_required(VERSION 3.25)
project(StarOS
  VERSION 0.1
  LANGUAGES C CXX ASM)

include(CTest)

# generate compiler_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# warning as error
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

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
add_dependencies(bootloadershared define_timestamp)

# mbr target
add_executable(mbr.elf
  ${SRC_DIR}/mbr.S
  ${SRC_DIR}/bootloader.S
  ${SRC_DIR}/bootloader_32.cpp
)
target_link_options(mbr.elf PRIVATE -T${SRC_DIR}/mbr.ld)
target_link_libraries(mbr.elf PUBLIC bootloadershared)
# asm file can't compile with warning as error option
set_property(TARGET mbr.elf PROPERTY COMPILE_WARNING_AS_ERROR OFF)

# in order to disable warning as error option
add_library(interrupt STATIC
  ${SRC_DIR}/interrupt_handlers.S
  ${SRC_DIR}/alltraps.S
)
set_property(TARGET interrupt PROPERTY COMPILE_WARNING_AS_ERROR OFF)

# kernel target
set(KERNEL_FILE staros_kernel.elf)
add_executable(${KERNEL_FILE}
  ${SRC_DIR}/kernel.h
  ${SRC_DIR}/linked_list.h
  ${SRC_DIR}/stdlib.cpp
  ${SRC_DIR}/memory_management.cpp
  ${SRC_DIR}/kernel.cpp
  ${SRC_DIR}/paging.cpp
  ${SRC_DIR}/interrupt.cpp
  ${SRC_DIR}/process.cpp
  ${SRC_DIR}/schedular.cpp
  ${SRC_DIR}/system_entry.cpp
  ${SRC_DIR}/PIC.cpp
  ${SRC_DIR}/ps2_keyboard.cpp
  ${SRC_DIR}/keyevent_table.cpp
)
target_link_options(${KERNEL_FILE} PRIVATE -T${SRC_DIR}/kernel.ld)
target_link_libraries(${KERNEL_FILE} PUBLIC bootloadershared interrupt)
target_compile_options(${KERNEL_FILE} PRIVATE -Wno-packed-bitfield-compat -save-temps)

target_include_directories(${KERNEL_FILE} PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )

set(DISK_IMG_FILENAME disk.img)
# Write MBR to disk image
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

set(QEMU_CMD_STR "qemu-system-i386 -m 5G -serial stdio -drive format=raw,file=${DISK_IMG_FILENAME}")
separate_arguments(QEMU_CMD NATIVE_COMMAND ${QEMU_CMD_STR})
# run qemu
add_custom_target(
  qemu
  DEPENDS disk.img
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMAND qemu-system-i386 -m 5G -serial stdio ${DISK_IMG_FILENAME}
)

# run gdbserver
add_custom_target(
  debug
  DEPENDS disk.img
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMAND qemu-system-i386 -d int -m 5G -serial stdio ${DISK_IMG_FILENAME} -s -S
)

add_library(c STATIC src_libc/system_call.cpp
  src_libc/test_system_calls.cpp
  src_libc/printf.cpp
  src_libc/string_utils.cpp
  src_libc/gets.cpp
  )

# ====================== user mode programs
set(USER_DIR ${PROJECT_SOURCE_DIR}/src_user)

file(GLOB USER_PROGRAM_SOURCES src_user/*.cpp)
foreach( USER_PROGRAM_SOURCE ${USER_PROGRAM_SOURCES} )
    # Cut off the file extension and directory path
    get_filename_component( USER_PROGRAM_NAME ${USER_PROGRAM_SOURCE} NAME_WE )
    add_executable( ${USER_PROGRAM_NAME} ${USER_PROGRAM_SOURCE} )
    # Make sure YourLib is linked to each app
    target_include_directories(${USER_PROGRAM_NAME} PUBLIC src_libc)
    target_link_libraries( ${USER_PROGRAM_NAME} c )
    target_link_options(${USER_PROGRAM_NAME} PRIVATE -T${USER_DIR}/userspace.ld)
    list(APPEND USER_PROGRAMS ${USER_PROGRAM_NAME})
endforeach( USER_PROGRAM_SOURCE ${USER_PROGRAM_SOURCES} )
message(${USER_PROGRAMS})

# make a disk image
# partition 200MB
# have two text file
set(USB_DEVICE /dev/sdb CACHE STRING "which disk to write into")
set(MOUNT_POINT /mnt/fat32_device)
set(FS_IMG /tmp/fs.img)
set(LOOP_DEVICE /dev/loop0)
# set(USER_PROGRAMS testadd)
add_custom_command(
  OUTPUT ${PROJECT_BINARY_DIR}/fs.img
  DEPENDS ${KERNEL_FILE} ${USER_PROGRAMS}
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}

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
  COMMAND sudo cp ${PROJECT_BINARY_DIR}/${USER_PROGRAMS} ${MOUNT_POINT}
  COMMAND sudo cp ${PROJECT_SOURCE_DIR}/root_dir/hello.txt ${MOUNT_POINT}
  COMMAND sudo cp ${PROJECT_SOURCE_DIR}/root_dir/world.txt ${MOUNT_POINT}
  COMMAND sudo cp ${PROJECT_SOURCE_DIR}/root_dir/large_hello.txt ${MOUNT_POINT}
  COMMAND sudo umount ${LOOP_DEVICE}p1
  COMMAND sudo rm -rf ${MOUNT_POINT}
  COMMAND sudo losetup -d ${LOOP_DEVICE}
  # directly manipulate image on project directory is extreamly slow
  COMMAND sudo cp /tmp/fs.img ${PROJECT_BINARY_DIR}/fs.img
  COMMAND echo "Done."
)

# define compilation timestamp macro
add_custom_target(
  define_timestamp
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  BYPRODUCTS ${PROJECT_BINARY_DIR}/Configs.h
  COMMAND ${PROJECT_SOURCE_DIR}/create_timestamp_macro.sh
)


# Write first 250MB to /dev/sdb
add_custom_target(
  burn
  DEPENDS disk.img
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMAND ${PROJECT_SOURCE_DIR}/burn2disk.sh ${USB_DEVICE}
)

# read first 200M from disk
add_custom_target(
  dump
  COMMAND sudo dd status=none if=${USB_DEVICE} of=/tmp/sdb_250M.img count=512000
  COMMAND hexdump -C /tmp/sdb_250M.img
)

# generate interrupt_handler.S
add_custom_command(
  OUTPUT ${SRC_DIR}/interrupt_handlers.S
  COMMAND ${SRC_DIR}/codegen_interrupt_handlers.py > ${SRC_DIR}/interrupt_handlers.S
  DEPENDS ${SRC_DIR}/codegen_interrupt_handlers.py
  WORKING_DIRECTORY ${SRC_DIR}
)

set(OUTPUT_STANDARD_FILE standard_log)
set(OUTPUT_TEST_FILE current_log)
# generate standard data
add_custom_target(
  gen_standard_test_data
  DEPENDS disk.img
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMAND ${QEMU_CMD} > ${PROJECT_SOURCE_DIR}/${OUTPUT_STANDARD_FILE}
)

# generate test data
add_custom_target(
  gen_test_data
  DEPENDS disk.img
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMAND ${QEMU_CMD} > ${OUTPUT_TEST_FILE}
)

# must generate test data first
# cmake --build . --target gen_test_data
add_test(NAME stdout_comparison
  COMMAND ${PROJECT_SOURCE_DIR}/stdout_comparison.sh
    ${PROJECT_SOURCE_DIR}/${OUTPUT_STANDARD_FILE}
    ${PROJECT_BINARY_DIR}/${OUTPUT_TEST_FILE})

include_directories(${PROJECT_SOURCE_DIR}/shared)