#include "console.h"
#include "cpu/isr.h"
#include "cpu/gdt.h"
#include "cpu/memlayout.h"
#include "drivers/keyboard.h"
#include "drivers/vga.h"
#include "drivers/ata.h"
#include "drivers/misc.h"
#include "drivers/pit.h"
#include "drivers/uart.h"
#include "fs/fs.h"
#include "lib/string.h"
#include "proc.h"
#include "kernel/mem.h"


void kmain() {
    freerange(P2V(1u<<20), P2V(2u<<20)); // 1MB - 2MB
    kvmalloc();  // map all of physical memory at KERNBASE
    freerange(P2V(2u<<20), P2V(PHYSTOP));

    load_gdt();
    init_keyboard();
    init_pit();
    uartinit();
    load_idt();
    sti();

    vga_clear_screen();
    printk("YABLOKO\n");

    printk("\n> ");
    while (1) {
        if (kbd_buf_size > 0 && kbd_buf[kbd_buf_size-1] == '\n') {
            if (!strncmp("halt\n", kbd_buf, kbd_buf_size)) {
                qemu_shutdown();
            } else if (!strncmp("work\n", kbd_buf, kbd_buf_size)) {
                for (int i = 0; i < 5; ++i) {
                    msleep(1000);
                    printk(".");
                }
            } else if (!strncmp("run ", kbd_buf, 4)) {
                kbd_buf[kbd_buf_size-1] = '\0';
                const char* cmd = kbd_buf + 4;
                run_elf(cmd);
            } else {
                printk("unknown command, try: halt | run CMD");
            }
            kbd_buf_size = 0;
            printk("\n> ");
        }
        asm("hlt");
    }
}
