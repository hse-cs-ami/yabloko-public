#pragma once
#include <stdint.h>

enum {
    IRQ0 = 32,
    IRQ1,
    IRQ2,
    IRQ3,
    IRQ4,
    IRQ5,
    IRQ6,
    IRQ7,
    IRQ8,
    IRQ9,
    IRQ10,
    IRQ11,
    IRQ12,
    IRQ13,
    IRQ14,
    IRQ15,
};

/* Struct which aggregates many registers.
 * It matches exactly the pushes on vectors.S. From the bottom:
 * - pushed by the processor automatically
 * - `push byte`s on the isr-specific code: error code, then int number
 * - segment registers
 * - all the registers by pusha
 */
typedef struct {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; /* Pushed by pusha. */
    uint32_t gs, fs, es, ds;
    uint32_t int_no, err_code; // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags;  // Pushed by the processor automatically

    uint32_t useresp, ss;      // Pushed by the processor for userspace interrupts
} registers_t;

void isr_install();

void isr_handler(registers_t *r);

typedef void (*isr_t)(registers_t *);

void register_interrupt_handler(uint8_t n, isr_t handler);

void load_idt();
void cli();
void sti();
