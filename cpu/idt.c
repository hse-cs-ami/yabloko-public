#include "isr.h"
#include "../drivers/port.h"
#include "../console.h"

enum {
    IDT_HANDLERS = 256,
};

typedef struct {
    uint16_t low_offset;
    uint16_t selector;
    uint8_t always0;
    uint8_t flags;
    uint16_t high_offset;
} __attribute__((packed)) idt_gate_t;

idt_gate_t idt[IDT_HANDLERS];

#define low_16(address) (uint16_t)((address) & 0xFFFF)
#define high_16(address) (uint16_t)(((address) >> 16) & 0xFFFF)

void set_idt_gate(int n, uint32_t handler) {
    idt[n].low_offset = low_16(handler);
    idt[n].selector = 0x08; // see GDT
    idt[n].always0 = 0;
    // 0x8E = 1  00 0 1  110
    //        P DPL 0 D Type
    idt[n].flags = 0x8E;
    idt[n].high_offset = high_16(handler);
}

// defined in vectors.S
extern const uint32_t default_handlers[];

void init_idt() {
    if (default_handlers[0] == 0) {
        panic("handler table empty\n");
    }
    for (int i = 0; i < IDT_HANDLERS; i++) {
        set_idt_gate(i, default_handlers[i]);
    }
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
    if (r->int_no < 32) {
        const char* msg = "Reserved";
        if (r->int_no < ARRLEN(exception_messages)) {
            msg = exception_messages[r->int_no];
        }
        panic(msg);
    }

    /* Handle the interrupt in a more modular way */
    if (interrupt_handlers[r->int_no] != 0) {
        isr_t handler = interrupt_handlers[r->int_no];
        handler(r);
    }

    // EOI
    if (r->int_no >= 40) {
        port_byte_out(0xA0, 0x20); /* follower */
    }
    if (r->int_no >= 32) {
        port_byte_out(0x20, 0x20); /* leader */
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
}

void cli() {
    asm("cli");
}

void sti() {
    asm("sti");
}
