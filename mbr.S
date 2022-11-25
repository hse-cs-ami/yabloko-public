#include "cpu/gdt.h"

    .code16
    .global _start
_start:
    mov %dl, boot_drive
    mov $banner, %si
    call print_string

    call get_drive_geometry
    call load_kernel
    call switch_to_32bit

    hlt
    jmp . // loop forever

get_drive_geometry:
    mov $8, %ah
    mov boot_drive, %dl
    int $0x13
    inc %dh  // number of heads
    mov %dh, disk_heads
    and 0x3f, %cl
    mov %cl, sectors_per_track
    ret


.equ ELF32_ENTRY_OFFSET, 0x18
.equ ELF32_PHDR_OFFSET, 0x1c
.equ ELF32_PHENTSIZE_OFFSET, ELF32_PHDR_OFFSET + 14
.equ ELF32_PHNUM_OFFSET, ELF32_PHENTSIZE_OFFSET + 2
.equ ELF32_PHDR_P_OFFSET, 4
.equ ELF32_PHDR_PTYPE_OFFSET, 0
.equ ELF32_PHDR_FILESZ_OFFSET, 4*4
.equ KERNEL_OFFSET, 0x1000

.equ PT_LOAD, 1

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

    mov KERNEL_OFFSET + ELF32_PHNUM_OFFSET, %si
read_segment:
    dec %si  // no offset to the first entry
    mov %si, %ax
    mulb KERNEL_OFFSET + ELF32_PHENTSIZE_OFFSET
    mov %ax, %di
    add KERNEL_OFFSET + ELF32_PHDR_OFFSET, %di
    // now di holds offset to the phentry
    mov KERNEL_OFFSET + ELF32_PHDR_PTYPE_OFFSET(%di), %ax
    cmp $PT_LOAD, %ax
    jnz read_segment // not a PT_LOAD segment
    mov KERNEL_OFFSET + ELF32_PHDR_FILESZ_OFFSET(%di), %ax
    test %ax, %ax
    jz read_segment // empty segment

    // now di holds offset to the last phentry loaded from file, ax its filesz

    add KERNEL_OFFSET + ELF32_PHDR_P_OFFSET(%di), %ax
    sub $0x1000, %ax // we won't load the header
    add $SECTOR_SIZE - 1, %ax
    shr $SECTOR_SHIFT, %ax // round up to sector count

    mov $SECTOR_BASE + MBR_SECTORS + ELFHDR_SECTORS, %cl //start after ELF header
    call bios_disk_read
    ret

bios_disk_read:
    // expects %al to specify number of sectors, %cl the initial sector
    xor %ah, %ah
    mov %ax, %si
    mov $0, %ch             // cylinder 0
    mov $0, %dh             // head 0
    mov $KERNEL_OFFSET, %bx // bx -> destination
    mov boot_drive, %dl     // dl -> disk
    mov $1, %al

1:
    mov $2, %ah             // read mode
    int $0x13
    jc fail
    add $SECTOR_SIZE, %bx
    inc %cl
    dec %si
    jnz 1b
    ret

fail:
    mov $read_error, %si
    call print_string
    hlt
    jmp .

switch_to_32bit:
    mov	$2, %al
    out	%al, $0x92          // enable A20

    cli                     // 1. disable interrupts
    lgdt gdt_descriptor     // 2. load GDT descriptor
    mov %cr0, %eax
    or $1, %eax             // 3. enable protected mode
    mov %eax, %cr0
    ljmp $SEG_KCODE << 3, $init_32bit // 4. far jump


.code32
init_32bit:
    mov $SEG_KDATA << 3, %ax      // 5. update segment registers
    mov %ax, %ds
    mov %ax, %ss
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    mov $KERN_STACK_BASE, %ebp      // 6. setup stack
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
read_error:
    .asciz "Read error\r\n"

    .balign 2
entry:
    .word 0
disk_heads:
    .byte 0
sectors_per_track:
    .byte 0

    .balign 4
gdt_start:
    .quad 0x0 // null descriptor
    SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)   # code seg
    SEG_ASM(STA_W, 0x0, 0xffffffff)         # data seg
gdt_end:

// GDT descriptor
gdt_descriptor:
    .word gdt_end - gdt_start - 1 // size (16 bit)
    .int  gdt_start               // address (32 bit)

    . = _start + 510     # pad to 510 bytes
    .byte 0x55, 0xaa     # boot sector magic value
