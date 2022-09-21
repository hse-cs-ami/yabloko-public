    .code16
    .global _start
_start:                   #точка входа
    mov $banner, %si
    call print_string

    jmp . // loop forever


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
banner:
    .asciz "YABLOKO bootloader started\r\n"

    . = _start + 510     # pad to 510 bytes
    .byte 0x55, 0xaa     # boot sector magic value
