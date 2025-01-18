#include "isr.h"
#include "gdt.h"
#include "memlayout.h"
#include "../syscall.h"
#include "../proc.h"
#include "../drivers/port.h"
#include "../console.h"

enum {
    IDT_HANDLERS = 256,
};

typedef struct {
    uint16_t low_offset;
    uint16_t selector;
    uint8_t always0;
    uint8_t type: 4;
    uint8_t s: 1;
    uint8_t dpl: 2;
    uint8_t p: 1;
    uint16_t high_offset;
} __attribute__((packed)) idt_gate_t;

idt_gate_t idt[IDT_HANDLERS];

#define low_16(address) (uint16_t)((address) & 0xFFFF)
#define high_16(address) (uint16_t)(((address) >> 16) & 0xFFFF)

#define STS_IG32    0xE     // 32-bit Interrupt Gate
#define STS_TG32    0xF     // 32-bit Trap Gate

void set_idt_gate(int n, _Bool istrap, uint32_t handler, uint8_t dpl) {
    idt[n].low_offset = low_16(handler);
    idt[n].selector = 0x08; // see GDT
    idt[n].always0 = 0;
    idt[n].type = istrap ? STS_TG32 : STS_IG32;
    idt[n].s = 0;
    idt[n].dpl = dpl;
    idt[n].p = 1;
    idt[n].high_offset = high_16(handler);
}

// defined in vectors.S
extern const uint32_t default_handlers[];

void init_idt() {
    if (default_handlers[0] == 0) {
        panic("handler table empty\n");
    }
    for (int i = 0; i < IDT_HANDLERS; i++) {
        set_idt_gate(i, 0, default_handlers[i], 0);
    }
    set_idt_gate(T_SYSCALL, 1, default_handlers[T_SYSCALL], DPL_USER);
}

const char * const exception_messages[] = {
    [0] = "Division By Zero",
    [1] = "Debug",
    [2] = "Non Maskable Interrupt",
    [3] = "Breakpoint",
    [4] = "Into Detected Overflow",
    [5] = "Out of Bounds",
    [6] = "Invalid Opcode",
    [7] = "No Coprocessor",

    [8] = "Double Fault",
    [9] = "Coprocessor Segment Overrun",
    [10] = "Bad TSS",
    [11] = "Segment Not Present",
    [12] = "Stack Fault",
    [13] = "General Protection Fault",
    [14] = "Page Fault",
    [15] = "Unknown Interrupt",

    [16] = "Coprocessor Fault",
    [17] = "Alignment Check",
    [18] = "Machine Check",
};

#define ARRLEN(a) (sizeof(a) / sizeof(a[0]))

static isr_t interrupt_handlers[IDT_HANDLERS];

void register_interrupt_handler(uint8_t i, isr_t handler) {
    interrupt_handlers[i] = handler;
}

void trap(registers_t *r) {
    // EOI
    if (r->int_no >= 40) {
        port_byte_out(0xA0, 0x20); /* follower */
    }
    if (r->int_no >= 32) {
        port_byte_out(0x20, 0x20); /* leader */
    }

    // Call registered handler
    if (interrupt_handlers[r->int_no] != 0) {
        isr_t handler = interrupt_handlers[r->int_no];
        handler(r);
        return;
    }

    if (r->int_no < 32) {
        const char* msg = "Reserved";
        if (r->int_no < ARRLEN(exception_messages)) {
            msg = exception_messages[r->int_no];
        }
        if (r->cs & 3) {
            // exception from user mode, kill offending process
            printk("Exception: ");
            printk(msg);
            printk("\n");
            killproc();
        }
        panic(msg);
    }
}

static void* get_userspace_ptr(uint32_t ptr) {
    if (ptr > KERNBASE) {
        return 0;
    }
    // FIXME: check if ptr is mapped and a valid 0-terminated string
    return (void*)(ptr);
}

static int handle_puts(const char* s) {
    if (!s) {
        return -1;
    }
    printk(s);
    return 0;
}

static void handle_syscall(registers_t* r) {
    switch (r->eax) {
        case SYS_exit:
            if (r->ebx == 0) {
                printk("* success\n");
            } else {
                printk("* failure\n");
            }
            killproc();
        case SYS_greet:
            printk("Hello world!\n");
            r->eax = 0;
            break;
        case SYS_putc:
            printk((const char[]){r->ebx, '\0'});
            r->eax = 0;
            break;
        case SYS_puts:
            r->eax = handle_puts(get_userspace_ptr(r->ebx));
            break;
        default:
            printk("Unknown syscall\n");
            r->eax = -1;
    }
}

static void init_pic() {
    // ICW1
    port_byte_out(0x20, 0x11);
    port_byte_out(0xA0, 0x11);

    // ICW2
    port_byte_out(0x21, 0x20);
    port_byte_out(0xA1, 0x28);

    // ICW3
    port_byte_out(0x21, 0x04);
    port_byte_out(0xA1, 0x02);

    // ICW4
    port_byte_out(0x21, 0x01);
    port_byte_out(0xA1, 0x01);

    // OCW1
    port_byte_out(0x21, 0x0);
    port_byte_out(0xA1, 0x0);
}

typedef struct {
    uint16_t limit;
    void* base;
} __attribute__((packed)) idt_register_t;

static idt_register_t idt_reg;

void load_idt() {
    init_idt();

    idt_reg.base = &idt;
    idt_reg.limit = sizeof(idt) - 1;
    asm("lidt (%0)" : : "r"(&idt_reg));

    init_pic();

    register_interrupt_handler(T_SYSCALL, handle_syscall);
}

void cli() {
    asm("cli");
}

void sti() {
    asm("sti");
}
