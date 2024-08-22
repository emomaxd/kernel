#!/bin/bash

# Usage message
usage() {
    echo "Usage: $0 /dev/sdX [OS_IMAGE]"
    echo "  /dev/sdX: The device name of the flash drive (e.g., /dev/sdb)"
    echo "  OS_IMAGE: Path to the OS image file to write to the flash drive"
    exit 1
}

# Check for required arguments
if [ "$#" -ne 2 ]; then
    usage
fi

DEVICE="$1"
OS_IMAGE="$2"

# Confirm the device name
echo "Warning: This script will erase all data on ${DEVICE}."
read -p "Are you sure you want to continue? (yes/no) " CONFIRM
if [ "$CONFIRM" != "yes" ]; then
    echo "Aborting."
    exit 1
fi

# Unmount all partitions of the device
echo "Unmounting partitions..."
umount ${DEVICE}?*

# Create a new partition table and format the partition
echo "Partitioning ${DEVICE}..."
(
echo o    # Create a new DOS partition table
echo n    # Add a new partition
echo p    # Primary partition
echo 1    # Partition number
echo      # Default first sector
echo      # Default last sector
echo a    # Mark partition as bootable
echo 1    # Select partition 1
echo w    # Write changes
) | fdisk ${DEVICE}

# Format the new partition as FAT32
PARTITION="${DEVICE}1"
echo "Formatting ${PARTITION} as FAT32..."
mkfs.vfat -F 32 ${PARTITION}

# Write the OS image to the partition
echo "Writing OS image to ${PARTITION}..."
dd if=${OS_IMAGE} of=${PARTITION} bs=512 seek=4 conv=fsync

# Inform the user of completion
echo "Done! ${DEVICE} is now prepared and the OS image has been written."

