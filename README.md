# Yet Another BootLoader, OS Kernel and Other stuff

## Quickstart:
```
$ ./setup.sh
$ make
```
## How to run using llvm

You can use this way even if you have windows. You need to install llvm and qemu. 
Check that executables `clang`, `ld.lld`, `qemu-system-i386` available from your terminal/console.

```
make LLVM=on
```

## How to debug in my favorite IDE

Start debug server using command `make debug-server` or `make debug-server-nox` if you don't want to see gui, and 
then connect using remote gdb option to localhost:1234 (symbols file is kernel.bin)

## Includes code from:
* https://github.com/mit-pdos/xv6-public
* https://github.com/FRosner/FrOS
* https://github.com/dhavalhirdhav/LearnOS
* https://wiki.osdev.org
