#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <path_to_iso> <output_device>"
    exit 1
fi

ISO_PATH=$1
OUTPUT_DEVICE=$2

# Validate the ISO file exists
if [ ! -f "$ISO_PATH" ]; then
    echo "Error: ISO file '$ISO_PATH' does not exist."
    exit 2
fi

# Validate the output device exists
if [ ! -b "$OUTPUT_DEVICE" ]; then
    echo "Error: Output device '$OUTPUT_DEVICE' does not exist or is not a block device."
    exit 3
fi

# Write the ISO to the output device
echo "Writing '$ISO_PATH' to '$OUTPUT_DEVICE'..."
sudo dd if="$ISO_PATH" of="$OUTPUT_DEVICE" bs=4M status=progress && sync

echo "Operation completed."
