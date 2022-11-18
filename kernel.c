asm(".asciz \"kernel start\"");

#include "vga.h"
#include "drivers/ata.h"
#include "drivers/misc.h"

void _start() {
    char buf[512];

    vga_clear_screen();
    vga_print_string("YABLOKO\n");

    read_sectors_ATA_PIO((uint32_t)buf, 10, 1);
    vga_print_string(buf);

    asm("hlt");
    qemu_shutdown();
}
