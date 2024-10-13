
# Kernel

## Overview

This project involves developing a custom operating system (OS) that includes a bootloader and a kernel. The OS is written in Assembly and C and is intended to run on both real hardware and virtual machines. The project features a basic bootloader using Limine.

## Features to come

- Dynamic Memory Allocation
- File System
- Process Scheduling
- User Space
- System Calls

## Building & Running

To run the OS in a virtual environment, simply execute:

```bash
make all & make run
```

## Burning into Flash

1. Identify the address of your flash drive (e.g., `/dev/sdX`).
2. Run the following command, this will create image.hdd file

    ```bash
    ./flash.sh
    ```
3. Use the `prepare_flash.sh` script with two command-line parameters:

    ```bash
    sudo ./prepare_flash.sh image.hdd /dev/sdX
    ```

4. Once the OS is burned onto the flash drive, you can boot it on real hardware.

---

![OS Output](git/OS_output_qemu.png)