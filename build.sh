#!/bin/bash

# Create necessary directories
mkdir -p build
mkdir -p bin

# Compiler and toolchain settings
CC="gcc"
LD="ld"
OBJCOPY="objcopy"
AS="as"

# File paths
BOOTLOADER_SRC="bootloader.asm"
KERNEL_SRC="kernel.c"
LINKER_SCRIPT="linker.ld"
BOOTLOADER_BIN="build/bootloader.bin"
KERNEL_BIN="build/kernel.bin"
IMAGE="bin/boot.img"

# Clean previous builds
clean() {
    echo "Cleaning previous builds..."
    rm -rf build/*
    rm -rf bin/*
}

# Compile the bootloader
build_bootloader() {
    echo "Compiling the bootloader..."
    $AS -o build/bootloader.o $BOOTLOADER_SRC
    $LD -o $BOOTLOADER_BIN -Ttext 0x7C00 --oformat binary build/bootloader.o
}

# Compile the kernel
build_kernel() {
    echo "Compiling the kernel..."
    $CC -ffreestanding -m32 -c -o build/kernel.o $KERNEL_SRC
    $LD -o build/kernel.bin -T $LINKER_SCRIPT --oformat binary build/kernel.o
}

# Create the final image file
create_image() {
    echo "Creating the image file..."
    cat $BOOTLOADER_BIN $KERNEL_BIN > $IMAGE
}

# Run the image in QEMU
run_qemu() {
    echo "Testing in QEMU..."
    qemu-system-i386 -drive format=raw,file=$IMAGE
}

# Main function to execute all steps
main() {
    clean
    build_bootloader
    build_kernel
    create_image
    run_qemu
}

# Execute the script
main
