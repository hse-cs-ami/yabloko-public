GDB=gdb

ifeq ($(shell uname -s),Darwin)
AS=x86_64-elf-as
LD=x86_64-elf-ld
CC=x86_64-elf-gcc
GDB=x86_64-elf-gdb
endif

CFLAGS = -fno-pic -ffreestanding -static -fno-builtin -fno-strict-aliasing \
		 -Wall -ggdb -m32 -Werror -fno-omit-frame-pointer
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

run: image.bin
	qemu-system-i386 -drive format=raw,file=$< -serial mon:stdio

run-nox: image.bin
	qemu-system-i386 -nographic -drive format=raw,file=$< -serial mon:stdio

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

debug: image.bin
	qemu-system-i386 -drive format=raw,file=$< -s -S &
	$(GDB) kernel.bin \
		-ex "target remote localhost:1234" \
		-ex "break _start" \
		-ex "continue"

debug-nox: image.bin
	qemu-system-i386 -nographic -drive format=raw,file=$< -s -S &
	$(GDB) kernel.bin \
		-ex "target remote localhost:1234" \
		-ex "break _start" \
		-ex "continue"

fs.img: kernel.bin tools/mkfs
	tools/mkfs $@ $<

LDFLAGS=-m elf_i386

user/%: user/%.o user/crt.o
	$(LD) $(LDFLAGS) -o $@ -Ttext 0x1000 $^

image.bin: mbr.bin fs.img
	cat $^ >$@

kernel.bin: kernel.o console.o drivers/vga.o drivers/uart.o
	$(LD) $(LDFLAGS) -o $@ -Ttext 0x1000 $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) -m32 -ffreestanding -c -g $^ -o $@

mbr.bin: mbr.o
	$(LD) -m elf_i386 -Ttext=0x7c00 --oformat=binary $^ -o $@

mbr.elf: mbr.o
	$(LD) -m elf_i386 -Ttext=0x7c00 $^ -o $@

clean:
	rm -f *.elf *.img *.bin *.o */*.o tools/mkfs

tools/%: tools/%.c
	gcc -Wall -Werror -g $^ -o $@
