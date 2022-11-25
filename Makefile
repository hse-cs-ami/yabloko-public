ifeq ($(shell uname -s),Darwin)
AS=x86_64-elf-as
LD=x86_64-elf-ld
CC=x86_64-elf-gcc
endif

run: image.bin
	qemu-system-i386 -drive format=raw,file=$< -serial mon:stdio

debug-preboot: image.bin mbr.elf
	qemu-system-i386 -drive format=raw,file=$< -s -S &
	x86_64-elf-gdb mbr.elf \
		-ex "set architecture i8086" \
		-ex "target remote localhost:1234" \
		-ex "break *0x7c00" \
		-ex "continue"

debug-boot: image.bin mbr.elf
	qemu-system-i386 -drive format=raw,file=$< -s -S &
	x86_64-elf-gdb mbr.elf \
		-ex "target remote localhost:1234" \
		-ex "break init_32bit" \
		-ex "continue"

debug: image.bin
	qemu-system-i386 -drive format=raw,file=$< -s -S &
	x86_64-elf-gdb kernel.bin \
		-ex "target remote localhost:1234" \
		-ex "break _start" \
		-ex "continue"

fs.img: kernel.bin tools/mkfs user/false
	tools/mkfs $@ $<

LDFLAGS=-m elf_i386

user/%: user/%.o user/crt.o
	$(LD) $(LDFLAGS) -o $@ -Ttext 0x101000 $^

image.bin: mbr.bin fs.img
	cat $^ >$@

kernel.bin: kernel.o console.o drivers/vga.o drivers/keyboard.o \
	string.o drivers/ata.o cpu/vectors.o cpu/idt.o cpu/gdt.o drivers/uart.o
	$(LD) $(LDFLAGS) -o $@ -Ttext 0x1000 $^

%.o: %.c
	$(CC) -m32 -ffreestanding -Wall -Werror -c -g $< -o $@

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
