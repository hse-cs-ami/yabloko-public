    .code16
    .global _start
_start:
    mov %dl, boot_drive
    mov $banner, %si
    call print_string

    call load_kernel
    call switch_to_32bit

    jmp . // loop forever


.equ KERNEL_OFFSET, 0x1000
load_kernel:
    mov $2, %ah             // read mode
    mov $2, %al             // sectors to read
    mov $2, %cl             // start from sector 2
    mov $0, %ch             // cylinder 0
    mov $0, %dh             // head 0
    mov $KERNEL_OFFSET, %bx // bx -> destination
    mov boot_drive, %dl     // dl -> disk
    int $0x13
    // no error checking, let's hope it works
    ret


switch_to_32bit:
    cli                     // 1. disable interrupts
    lgdt gdt_descriptor    // 2. load GDT descriptor
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

    call KERNEL_OFFSET      // 7. jump to the kernel
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
