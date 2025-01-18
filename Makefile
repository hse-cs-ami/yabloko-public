GDB=gdb
OBJCOPY=objcopy
CAT=cat

ifeq ($(OS),Windows_NT)
	CAT=type
else
	OS=$(shell uname -s)
endif

ifeq ($(OS),Darwin)
AS=x86_64-elf-as
LD=x86_64-elf-ld
CC=x86_64-elf-gcc
GDB=x86_64-elf-gdb
OBJCOPY=x86_64-elf-objcopy
endif

CFLAGS = -fno-pic -ffreestanding -static -fno-builtin -fno-strict-aliasing \
		 -mno-sse \
		 -I. \
		 -Wall -ggdb -m32 -Werror -fno-omit-frame-pointer
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
ASMFLAGS = -m32 -ffreestanding -c -g -I.

ifeq ($(LLVM),on)

ifeq ($(OS),Darwin)
	LD=PATH=/usr/local/opt/llvm/bin:"$(PATH)" ld.lld
else
	LD=ld.lld
endif

CC=clang
CFLAGS += -target elf-i386
ASMFLAGS += -target elf-i386
LDKERNELFLAGS = --script=script.ld
endif

OBJECTS = ./kernel/kstart.o ./kernel.o ./console.o ./drivers/vga.o ./drivers/uart.o ./drivers/keyboard.o \
	./cpu/idt.o ./cpu/gdt.o ./cpu/swtch.o ./cpu/vectors.o ./kernel/mem.o ./proc.o ./lib/string.o \
	./fs/fs.o ./drivers/ata.o ./lib/string.o ./proc.o ./drivers/pit.o ./kernel/vm.o

run: image.bin
	qemu-system-i386 -drive format=raw,file=$< -serial mon:stdio

run-nox: image.bin
	qemu-system-i386 -nographic -drive format=raw,file=$< -serial mon:stdio

ejudge.sh: image.bin
	echo >$@ "#!/bin/sh"
	echo >>$@ "base64 -d <<===EOF | gunzip >image.bin"
	gzip <$^ | base64 >>$@
	echo >>$@ "===EOF"
	echo >>$@ "exec qemu-system-i386 -nographic -drive format=raw,file=image.bin -serial mon:stdio"
	chmod +x $@

diag:
	-$(UNAME) -a
	-$(CC) --version
	-$(LD) -v
	-gcc --version
	-ld -v

debug-boot-nox: image.bin mbr.elf
	qemu-system-i386 -nographic -drive format=raw,file=$< -s -S &
	$(GDB) mbr.elf \
		-ex "set architecture i8086" \
		-ex "target remote localhost:1234" \
		-ex "break *0x7c00" \
		-ex "continue"

debug-boot: image.bin mbr.elf
	qemu-system-i386 -drive format=raw,file=$< -s -S &
	$(GDB) mbr.elf \
		-ex "set architecture i8086" \
		-ex "target remote localhost:1234" \
		-ex "break *0x7c00" \
		-ex "continue"

debug-server: image.bin
	qemu-system-i386 -drive format=raw,file=$< -s -S

debug-server-nox: image.bin
	qemu-system-i386 -nographic -drive format=raw,file=$< -s -S

debug: image.bin
	qemu-system-i386 -drive format=raw,file=$< -s -S &
	$(GDB) kernel.bin \
		-ex "target remote localhost:1234" \
		-ex "break kmain" \
		-ex "continue"

debug-nox: image.bin
	qemu-system-i386 -nographic -drive format=raw,file=$< -s -S &
	$(GDB) kernel.bin \
		-ex "target remote localhost:1234" \
		-ex "break _start" \
		-ex "continue"

fs.img: ./kernel.bin ./tools/mkfs ./user/false ./user/greet ./user/div0 ./user/shout
	./tools/mkfs $@ $< ./user/false ./user/greet ./user/div0 ./user/shout

LDFLAGS=-m elf_i386

user/%: user/%.o user/crt.o
	$(LD) $(LDFLAGS) -o $@ -Ttext 0x401000 $^

kernel.bin: $(OBJECTS)
	$(LD) $(LDFLAGS) $(LDKERNELFLAGS) -o $@ -Ttext 0x80009000 $^

bootmain.o: bootmain.c
	$(CC) $(CFLAGS) -Os -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(ASMFLAGS) $^ -o $@

image.bin: mbr.elf tools/mbrpad fs.img
	$(OBJCOPY) -S -O binary -j .text $< $@
	tools/mbrpad $@ fs.img

mbr.raw: mbr.o bootmain.o
	$(LD) -N -m elf_i386 -Ttext=0x7c00 --oformat=binary $^ -o $@

mbr.elf: mbr.o bootmain.o
	$(LD) -N -m elf_i386 -Ttext=0x7c00 $^ -o $@

clean:
	rm -f *.elf *.img *.bin *.raw *.o */*.o tools/mkfs ejudge.sh

tools/%: tools/%.c
	gcc -Wall -Werror -g $^ -o $@
