asm(".asciz \"kernel start\"");

#include "console.h"
#include "cpu/isr.h"
#include "drivers/keyboard.h"
#include "drivers/vga.h"
#include "drivers/ata.h"
#include "drivers/misc.h"

void _start() {
    init_keyboard();
    load_idt();
    sti();
    char buf[512];

    vga_clear_screen();
    printk("YABLOKO\n");

    read_sectors_ATA_PIO((uint32_t)buf, 10, 1);
    printk(buf);

    while (1) {
        asm("pause");
    }
    asm("hlt");
    qemu_shutdown();
}
