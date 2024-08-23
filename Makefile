# Paths and cross-compiler settings
PREFIX := $(HOME)/opt/cross
TARGET := i686-elf
CC := $(PREFIX)/bin/$(TARGET)-gcc
LD := $(PREFIX)/bin/$(TARGET)-ld
AS := nasm
QEMU := qemu-system-x86_64

# Build directories and files
BUILD_DIR := build
BIN_DIR := $(BUILD_DIR)
SRC_DIR := src
INCLUDE_DIR := include

# Source files
BOOT_SRC := $(SRC_DIR)/arch/x86/boot.asm
KERNEL_ENTRY_SRC := $(SRC_DIR)/arch/x86/kernel_entry.asm
KERNEL_SRC := $(SRC_DIR)/kernel.c
ZEROES_SRC := $(SRC_DIR)/arch/x86/zeroes.asm

# Collect all .c source files and create object file paths
ALL_SRC := $(shell find $(SRC_DIR) -type f -name '*.c')
ALL_OBJS := $(patsubst $(SRC_DIR)/%, $(BIN_DIR)/%, $(ALL_SRC:.c=.o))

# Output files
BOOT_BIN := $(BIN_DIR)/boot.bin
KERNEL_ENTRY_OBJ := $(BIN_DIR)/kernel_entry.o
KERNEL_OBJ := $(BIN_DIR)/kernel.o
ZEROES_BIN := $(BIN_DIR)/zeroes.bin
FULL_KERNEL_BIN := $(BIN_DIR)/full_kernel.bin
OS_BIN := $(BIN_DIR)/OS.bin

# Compiler and linker flags
CFLAGS := -ffreestanding -m32 -g -I$(INCLUDE_DIR)
LDFLAGS := -T $(SRC_DIR)/arch/x86/linker.ld --oformat binary

# Phony targets
.PHONY: all clean run $(BUILD_DIR)

# Build the OS
all: $(BUILD_DIR) $(OS_BIN)

# Create build directories if they do not exist
$(BUILD_DIR):
	mkdir -p $(BIN_DIR)

# Assemble and compile
$(BOOT_BIN): $(BOOT_SRC)
	$(AS) $< -f bin -o $@

$(KERNEL_ENTRY_OBJ): $(KERNEL_ENTRY_SRC)
	$(AS) $< -f elf -o $@

$(KERNEL_OBJ): $(KERNEL_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(ZEROES_BIN): $(ZEROES_SRC)
	$(AS) $< -f bin -o $@

# Compile all .c files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D) # Ensure the directory exists
	$(CC) $(CFLAGS) -c $< -o $@

# Link the kernel
$(FULL_KERNEL_BIN): $(KERNEL_ENTRY_OBJ) $(KERNEL_OBJ) $(ALL_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

# Create the OS image by concatenating the bootloader, kernel, and zeroes
$(OS_BIN): $(BOOT_BIN) $(FULL_KERNEL_BIN) $(ZEROES_BIN)
	cat $^ > $@

# Run the OS in QEMU
run: $(OS_BIN)
	$(QEMU) -drive format=raw,file=$<

# Clean the build directory
clean:
	rm -rf $(BUILD_DIR)
