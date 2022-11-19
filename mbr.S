    .code16
    .global _start
_start:
    mov %dl, boot_drive
    mov $banner, %si
    call print_string

    call load_kernel
    call switch_to_32bit

    jmp . // loop forever


.equ ELF32_ENTRY_OFFSET, 0x18
.equ ELF32_PHDR_OFFSET, 0x1c
.equ ELF32_PHDR_FILESZ_OFFSET, 4*4
.equ KERNEL_OFFSET, 0x1000

.equ MBR_SECTORS, 2
.equ SECTOR_BASE, 1
.equ ELFHDR_SECTORS, 8

.equ SECTOR_SIZE, 512
.equ SECTOR_SHIFT, 9

load_kernel:
    mov $1, %al // sectors to read
    mov $SECTOR_BASE + MBR_SECTORS, %cl // start after MBR
    call bios_disk_read

    mov KERNEL_OFFSET + ELF32_ENTRY_OFFSET, %si
    mov %si, entry // store entry point

    mov KERNEL_OFFSET + ELF32_PHDR_OFFSET, %di
    mov KERNEL_OFFSET + ELF32_PHDR_FILESZ_OFFSET(%di), %ax
    sub $0x1000, %ax // we won't load the header
    add $SECTOR_SIZE - 1, %ax
    shr $SECTOR_SHIFT, %ax // round up to sector count

    mov $SECTOR_BASE + MBR_SECTORS + ELFHDR_SECTORS, %cl //start after ELF header
    call bios_disk_read
    ret

bios_disk_read:
    // expects %al to specify number of sectors, %cl the initial sector
    mov $2, %ah             // read mode
    mov $0, %ch             // cylinder 0
    mov $0, %dh             // head 0
    mov $KERNEL_OFFSET, %bx // bx -> destination
    mov boot_drive, %dl     // dl -> disk
    int $0x13
    // no error checking, let's hope it works
    ret


switch_to_32bit:
    mov	$2, %al
    out	%al, $0x92          // enable A20

    cli                     // 1. disable interrupts
    lgdt gdt_descriptor     // 2. load GDT descriptor
    mov %cr0, %eax
    or $1, %eax             // 3. enable protected mode
    mov %eax, %cr0
    ljmp $CODE_SEG, $init_32bit // 4. far jump


.code32
init_32bit:
    mov $DATA_SEG, %ax      // 5. update segment registers
    mov %ax, %ds
    mov %ax, %ss
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    mov $0x90000, %ebp      // 6. setup stack
    mov %ebp, %esp

    movzwl entry, %esi
    call *%esi              // 7. jump to the kernel
    jmp .                   // 8. loop forever


.code16
print_string:
    mov $0x0e, %ah  // "teletype output"
repeat:
    lodsb           // equivalent to mov (%si), %al; inc %si

    test %al, %al
    je done

    int $0x10       // bios interrupt
    jmp repeat
done:
    ret

    . = _start + 256     # pad to 256 bytes
boot_drive:
    .byte 0
banner:
    .asciz "YABLOKO bootloader started\r\n"

    .balign 2
entry:
    .word 0

    .balign 4
gdt_start:
    .quad 0x0 // null descriptor

// code segment descriptor
gdt_code:
    .word 0xffff    // segment length, bits 0-15
    .word 0x0       // segment base, bits 0-15
    .byte 0x0       // segment base, bits 16-23
    .byte 0b10011010 // flags (8 bits)
    .byte 0b11001111 // flags (4 bits) + segment length, bits 16-19
    .byte 0x0       // segment base, bits 24-31

// data segment descriptor
gdt_data:
    .word 0xffff    // segment length, bits 0-15
    .word 0x0       // segment base, bits 0-15
    .byte 0x0       // segment base, bits 16-23
    .byte 0b10010010 // flags (8 bits)
    .byte 0b11001111 // flags (4 bits) + segment length, bits 16-19
    .byte 0x0       // segment base, bits 24-31

gdt_end:

// GDT descriptor
gdt_descriptor:
    .word gdt_end - gdt_start - 1 // size (16 bit)
    .int  gdt_start               // address (32 bit)

.equ CODE_SEG, gdt_code - gdt_start
.equ DATA_SEG, gdt_data - gdt_start

    . = _start + 510     # pad to 510 bytes
    .byte 0x55, 0xaa     # boot sector magic value
