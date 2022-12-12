asm(".asciz \"kernel start\"");

#include "console.h"
#include "cpu/isr.h"
#include "cpu/gdt.h"
#include "drivers/keyboard.h"
#include "drivers/vga.h"
#include "drivers/ata.h"
#include "drivers/misc.h"
#include "drivers/uart.h"
#include "fs/fs.h"

void _start() {
    load_gdt();
    init_keyboard();
    uartinit();
    load_idt();
    sti();
    char buf[4096 + 512];

    vga_clear_screen();
    printk("YABLOKO\n");

    if (read_file("kernel.bin", buf, sizeof(buf)) == 0) {
        printk(buf + 4096);
    } else {
        printk("failed to read file\n");
    }

    while (1) {
        asm("hlt");
    }
    qemu_shutdown();
}
