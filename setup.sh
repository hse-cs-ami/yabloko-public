#!/bin/sh -x

if [ `uname` = Darwin ]; then
    brew install \
        x86_64-elf-binutils x86_64-elf-gcc \
        x86_64-elf-gdb qemu lld
elif [ `uname` = Linux ]; then
    sudo apt-get update
    sudo apt-get install qemu-system-x86
fi
