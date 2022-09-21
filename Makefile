AS=x86_64-elf-as
LD=x86_64-elf-ld

run: mbr.bin
	qemu-system-i386 -drive format=raw,file=$<

%.o: %.S
	$(AS) $^ -o $@

mbr.bin: mbr.o
	$(LD) -Ttext=0x7c00 --oformat=binary $^ -o $@

