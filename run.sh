# Export PATH for cross-compiler tools
export PATH=$PATH:/usr/local/i386elfgcc/bin
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

# Assemble and compile the components
nasm "boot.asm" -f bin -o "Binaries/boot.bin"
nasm "kernel_entry.asm" -f elf -o "Binaries/kernel_entry.o"
i686-elf-gcc -ffreestanding -m32 -g -c "kernel.cpp" -o "Binaries/kernel.o"
nasm "zeroes.asm" -f bin -o "Binaries/zeroes.bin"

# Link the kernel with the linker script
i686-elf-ld -o "Binaries/full_kernel.bin" -T linker.ld "Binaries/kernel_entry.o" "Binaries/kernel.o" --oformat binary

# Concatenate bootloader, kernel, and zeroes into a single OS image
cat "Binaries/boot.bin" "Binaries/full_kernel.bin" "Binaries/zeroes.bin" > "Binaries/OS.bin"

# Run the OS image in QEMU
qemu-system-x86_64 -drive format=raw,file="Binaries/OS.bin"
