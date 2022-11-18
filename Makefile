AS=x86_64-elf-as
LD=x86_64-elf-ld
CC=x86_64-elf-gcc

run: image.bin
	qemu-system-i386 -drive format=raw,file=$<

debug-boot: image.bin mbr.elf
	qemu-system-i386 -drive format=raw,file=$< -s -S &
	x86_64-elf-gdb mbr.elf \
		-ex "set architecture i386" \
		-ex "target remote localhost:1234" \
		-ex "break init_32bit" \
		-ex "continue"

debug: image.bin
	qemu-system-i386 -drive format=raw,file=$< -s -S &
	x86_64-elf-gdb kernel.bin \
		-ex "target remote localhost:1234" \
		-ex "break _start" \
		-ex "continue"

image.bin: mbr.bin kernel.bin
	cat $^ >$@

kernel.bin: kernel.o vga.o string.o drivers/ata.o
	$(LD) -m elf_i386 -o $@ -Ttext 0x1000 $^

%.o: %.c
	$(CC) -m32 -ffreestanding -c -g $< -o $@

%.o: %.S
	$(AS) $^ -g -o $@

mbr.bin: mbr.o
	$(LD) -Ttext=0x7c00 --oformat=binary $^ -o $@

mbr.elf: mbr.o
	$(LD) -Ttext=0x7c00 $^ -o $@

clean:
	rm *.bin *.o
