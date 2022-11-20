AS=x86_64-elf-as
LD=x86_64-elf-ld
CC=x86_64-elf-gcc

run: image.bin
	qemu-system-i386 -drive format=raw,file=$<

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

fs.img: kernel.bin tools/mkfs
	tools/mkfs $@ $<

image.bin: mbr.bin fs.img
	cat $^ >$@

kernel.bin: kernel.o console.o drivers/vga.o drivers/keyboard.o string.o drivers/ata.o cpu/vectors.o cpu/idt.o
	$(LD) -m elf_i386 -o $@ -Ttext 0x1000 $^

%.o: %.c
	$(CC) -m32 -ffreestanding -Wall -Werror -c -g $< -o $@

%.o: %.S
	$(AS) --32 -g $^ -o $@

mbr.bin: mbr.o
	$(LD) -m elf_i386 -Ttext=0x7c00 --oformat=binary $^ -o $@

mbr.elf: mbr.o
	$(LD) -m elf_i386 -Ttext=0x7c00 $^ -o $@

clean:
	rm -f *.elf *.bin *.o */*.o tools/mkfs

tools/%: tools/%.c
	gcc -Wall -Werror -g $^ -o $@
