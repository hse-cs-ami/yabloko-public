#pragma once

#define STA_X     0x8       // Executable segment
#define STA_W     0x2       // Writeable (non-executable segments)
#define STA_R     0x2       // Readable (executable segments)

// System segment type bits
#define STS_T32A    0x9     // Available 32-bit TSS
#define STS_IG32    0xE     // 32-bit Interrupt Gate
#define STS_TG32    0xF     // 32-bit Trap Gate

#define DPL_USER 3

#define FL_IF           0x00000200

#define SEG_KCODE 1
#define SEG_KDATA 2
#define SEG_UCODE 3
#define SEG_UDATA 4
#define SEG_TSS   5

#define SEG_ASM(type,base,lim)                                  \
        .word (((lim) >> 12) & 0xffff), ((base) & 0xffff);      \
        .byte (((base) >> 16) & 0xff), (0x90 | (type)),         \
                (0xC0 | (((lim) >> 28) & 0xf)), (((base) >> 24) & 0xff)

#define USER_BASE       0x400000 // 4 MB
#define USER_STACK_BASE 0xf00000 // 15 MB
#define KERN_STACK_BASE  0x90000

#ifndef __ASSEMBLER__
#include "kernel/mem.h"
typedef unsigned uint;
typedef unsigned short ushort;

struct taskstate {
    uint link;         // Old ts selector
    uint esp0;         // Stack pointers and segment selectors
    ushort ss0;        //   after an increase in privilege level
    ushort padding1;
    uint *esp1;
    ushort ss1;
    ushort padding2;
    uint *esp2;
    ushort ss2;
    ushort padding3;
    void *cr3;         // Page directory base
    uint *eip;         // Saved state from last task switch
    uint eflags;
    uint eax;          // More saved state (registers)
    uint ecx;
    uint edx;
    uint ebx;
    uint *esp;
    uint *ebp;
    uint esi;
    uint edi;
    ushort es;         // Even more saved state (segment selectors)
    ushort padding4;
    ushort cs;
    ushort padding5;
    ushort ss;
    ushort padding6;
    ushort ds;
    ushort padding7;
    ushort fs;
    ushort padding8;
    ushort gs;
    ushort padding9;
    ushort ldt;
    ushort padding10;
    ushort t;          // Trap on task switch
    ushort iomb;       // I/O map base address
};

void load_gdt();
void switchuvm(struct taskstate *tss, void* esp, pde_t *pgdir);
#endif
