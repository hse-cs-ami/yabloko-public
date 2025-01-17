#include "gdt.h"
#include "x86.h"
#include "memlayout.h"
#include "../lib/string.h"
#include "kernel/mem.h"

#include <stdint.h>

struct seg_desc_t {
    uint16_t lim_15_0;      // Low bits of segment limit
    uint16_t base_15_0;     // Low bits of segment base address
    uint8_t base_23_16;     // Middle bits of segment base address
    uint8_t type : 4;       // Segment type (see STA_ constants)
    uint8_t s : 1;          // 0 = system, 1 = application
    uint8_t dpl : 2;        // Descriptor Privilege Level
    uint8_t p : 1;          // Present
    uint8_t lim_19_16 : 4;  // High bits of segment limit
    uint8_t avl : 1;        // Unused (available for software use)
    uint8_t rsv1 : 1;       // Reserved
    uint8_t db : 1;         // 0 = 16-bit segment, 1 = 32-bit segment
    uint8_t g : 1;          // Granularity: limit scaled by 4K when set
    uint8_t base_31_24;     // High bits of segment base address
} __attribute__((packed));

#define SEG(type, base, lim, dpl) (struct seg_desc_t)    \
{ ((lim) >> 12) & 0xffff, (uint)(base) & 0xffff,      \
  ((uint)(base) >> 16) & 0xff, type, 1, dpl, 1,       \
  (uint)(lim) >> 28, 0, 0, 1, 1, (uint)(base) >> 24 }
#define SEG16(type, base, lim, dpl) (struct seg_desc_t)  \
{ (lim) & 0xffff, (uint)(base) & 0xffff,              \
  ((uint)(base) >> 16) & 0xff, type, 1, dpl, 1,       \
  (uint)(lim) >> 16, 0, 0, 1, 0, (uint)(base) >> 24 }

struct seg_desc_t seg_desc[6];

void init_seg_desc() {
    seg_desc[SEG_KCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, 0);
    seg_desc[SEG_KDATA] = SEG(STA_W,       0, 0xffffffff, 0);
    seg_desc[SEG_UCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, DPL_USER);
    seg_desc[SEG_UDATA] = SEG(STA_W,       0, 0xffffffff, DPL_USER);
}

struct gdt_desc_t {
    uint16_t size;
    void* ptr;
} __attribute__((packed));

void load_gdt() {
    init_seg_desc();

    struct gdt_desc_t gdt_desc;
    gdt_desc.size = sizeof(seg_desc) - 1;
    gdt_desc.ptr = seg_desc;
    asm("lgdt %0": : "m"(gdt_desc));
}

void switchuvm(struct taskstate *tss, void* esp, pde_t *pgdir) {
    memset(tss, 0, sizeof(*tss));
    seg_desc[SEG_TSS] = SEG16(STS_T32A, tss, sizeof(*tss)-1, 0);
    seg_desc[SEG_TSS].s = 0;
    tss->ss0 = SEG_KDATA << 3;
    tss->esp0 = (uint)esp;
    // setting IOPL=0 in eflags *and* iomb beyond the tss segment limit
    // forbids I/O instructions (e.g., inb and outb) from user space
    tss->iomb = (ushort) 0xFFFF;

    asm("ltr %0": : "r"((ushort)(SEG_TSS << 3)));

    lcr3(V2P(pgdir));
}
