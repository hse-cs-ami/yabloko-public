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
#include "string.h"

void _start() {
    load_gdt();
    init_keyboard();
    uartinit();
    load_idt();
    sti();

    char *buf = (char*)(16 << 20);

    vga_clear_screen();
    printk("YABLOKO\n");

    if (read_file("kernel.bin", buf, 4096 + sector_size) > 0) {
        printk(buf + 4096);
    } else {
        printk("failed to read file\n");
    }
    printk("\n> ");

    while (1) {
        if (kbd_buf_size > 0 && kbd_buf[kbd_buf_size-1] == '\n') {
            if (!strncmp("halt\n", kbd_buf, kbd_buf_size)) {
                qemu_shutdown();
            } else {
                printk("unknown command, try: halt\n> ");
            }
            kbd_buf_size = 0;
        }
        asm("hlt");
    }
}
