# Paths and cross-compiler settings
PREFIX := $(HOME)/opt/cross
TARGET := i686-elf
CC := $(PREFIX)/bin/$(TARGET)-gcc
AS := $(PREFIX)/bin/$(TARGET)-as
LD := $(PREFIX)/bin/$(TARGET)-ld
QEMU := qemu-system-x86_64

# Build directories and files
BUILD_DIR := build
SRC_DIR := src
INCLUDE_DIR := include

# Source files
BOOT_SRC := $(SRC_DIR)/arch/x86/boot.s
LINKER_SCRIPT := $(SRC_DIR)/arch/x86/linker.ld

# Collect all .c source files and create object file paths
ALL_SRC := $(shell find $(SRC_DIR) -type f -name '*.c')
ALL_OBJS := $(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/%, $(ALL_SRC:.c=.o))

# Object files
BOOT_OBJ := $(BUILD_DIR)/boot.o

# Output files
OS_BIN := $(BUILD_DIR)/OS.bin

# Compiler and linker flags
CFLAGS := -ffreestanding -O2 -std=gnu99 -Wall -Wextra -I$(INCLUDE_DIR)
LDFLAGS := -T $(LINKER_SCRIPT) -ffreestanding -O2 -nostdlib -lgcc

# Phony targets
.PHONY: all clean run iso

# Build the OS
all: $(BUILD_DIR) $(OS_BIN)

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Assemble the bootloader
$(BOOT_OBJ): $(BOOT_SRC) | $(BUILD_DIR)
	$(AS) $< -o $@

# Compile all .c files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(@D) # Ensure the directory exists
	$(CC) $(CFLAGS) -c $< -o $@

# Link the kernel and bootloader into the final OS binary
$(OS_BIN): $(BOOT_OBJ) $(ALL_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Run the OS in QEMU and create ISO image
run: $(OS_BIN) iso
	$(QEMU) -cdrom OS.iso

# Create ISO image
iso:
	./scripts/iso.sh

# Clean the build directory
clean:
	rm -rf $(BUILD_DIR)
