AS=x86_64-elf-as
LD=x86_64-elf-ld
CC=x86_64-elf-gcc

run: image.bin
	qemu-system-i386 -drive format=raw,file=$<

debug: image.bin
	qemu-system-i386 -drive format=raw,file=$< -s -S &
	i386-elf-gdb \
		-ex "target remote localhost:1234" \
		-ex "set architecture i8086" \
		-ex "break *0x7c00" \
		-ex "continue"

image.bin: mbr.bin kernel.bin
	cat $^ >$@

kernel.bin: kernel.o vga.o
	$(LD) -m elf_i386 -o $@ -Ttext 0x1000 $^ --oformat binary

%.o: %.c
	$(CC) -m32 -ffreestanding -c $< -o $@

%.o: %.S
	$(AS) $^ -o $@

mbr.bin: mbr.o
	$(LD) -Ttext=0x7c00 --oformat=binary $^ -o $@

clean:
	rm *.bin *.o
