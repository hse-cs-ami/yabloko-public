asm(".asciz \"kernel start\"");

#include "console.h"
#include "cpu/isr.h"
#include "cpu/gdt.h"
#include "drivers/keyboard.h"
#include "drivers/vga.h"
#include "drivers/ata.h"
#include "drivers/misc.h"
#include "drivers/uart.h"

void _start() {
    load_gdt();
    init_keyboard();
    uartinit();
    load_idt();
    sti();
    char buf[512];

    vga_clear_screen();
    printk("YABLOKO\n");

    read_sectors_ATA_PIO((uint32_t)buf, 10, 1);
    printk(buf);

    while (1) {
        asm("hlt");
    }
    qemu_shutdown();
}
