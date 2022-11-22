#pragma once

#define STA_X     0x8       // Executable segment
#define STA_W     0x2       // Writeable (non-executable segments)
#define STA_R     0x2       // Readable (executable segments)

#define DPL_USER 3

#define SEG_KCODE 1
#define SEG_KDATA 2
#define SEG_UCODE 3
#define SEG_UDATA 4
#define SEG_TSS   5

#define USER_BASE 0x400000 // 4 MB

#ifndef __ASSEMBLER__
void load_gdt();
#endif
