AS=x86_64-elf-as
LD=x86_64-elf-ld

run: mbr.bin
	qemu-system-i386 -drive format=raw,file=$<

debug: mbr.bin
	qemu-system-i386 -drive format=raw,file=$< -s -S &
	i386-elf-gdb \
		-ex "target remote localhost:1234" \
		-ex "set architecture i8086" \
		-ex "break *0x7c00" \
		-ex "continue"

%.o: %.S
	$(AS) $^ -o $@

mbr.bin: mbr.o
	$(LD) -Ttext=0x7c00 --oformat=binary $^ -o $@

clean:
	rm *.bin *.o
