#!/bin/bash

# Dizinleri oluştur
mkdir -p isodir/boot/grub

# Kernel bin dosyasını ve grub.cfg dosyasını belirtilen dizinlere kopyala
cp build/OS.bin isodir/boot/OS.bin
cp grub.cfg isodir/boot/grub/grub.cfg

# GRUB ile bootable ISO oluştur
grub-mkrescue -o OS.iso isodir

echo "ISO imajı başarıyla oluşturuldu: OS.iso"
