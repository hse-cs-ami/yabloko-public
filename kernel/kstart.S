#include "cpu/memlayout.h"

/*
    Memory layout at this point (see https://wiki.osdev.org/Memory_Map_(x86) for more details):
       0x00500 - 0x08fff: usable memory
       0x09000 – 0x14fff: kernel code and global data
       0x15000 - 0x7ffff: usable memory
       0x80000 - 0xfffff: BDA and upper memory
       0x100000 - 0x8000000 (1 MiB - 128 MiB): usable memory
*/
    .intel_syntax noprefix
    .global _start
    .asciz "kernel start\n"
_start:
.code32
    mov dword ptr [patch_addr - KERNBASE], offset gdt32_begin - KERNBASE
    lgdt gdtr32 - KERNBASE

    // zero our PD at 0x1000 and PT at 0x2000
    xor eax, eax
    mov ecx, 2048
    rep stosd

    // Set up the page table for the lowest 4 MiB: PD[0] -> PT
    mov dword ptr [0x1000], 0x2000 | 0x3

    // KERNBASE = 0x8000_0000
    // Use the same page table for the first 4 MiB after KERNBASE: PD[0x200] -> PT
    mov dword ptr [0x1000 + ((KERNBASE >> 22) * 4)], 0x2000 | 0x3

    // Enable 4 MiB pages
    mov eax, cr4
    or eax, 0x10        // Set the PSE bit (bit 4)
    mov cr4, eax

    // Identity map the first 2 MiB
    mov edi, 0x2000      // Start of the PT
    mov ebx, 0x00000003  // Present, R/W
    mov ecx, 512         // 2 MiB / 4 KiB = 512
SetEntry:
    mov dword ptr [edi], ebx
    add ebx, 0x1000
    add edi, 4
    loop SetEntry

    // Load physical address of PD into CR3
    mov edi, 0x1000
    mov cr3, edi

    // Enable paging
    mov eax, cr0
    or eax, 1 << 31      // Set the PG bit
    mov cr0, eax

    .att_syntax
    jmp $0x8, $loadseg - KERNBASE
    .intel_syntax noprefix

loadseg:
    // jump to the high half
    add esp, KERNBASE
    lea eax, kmain
    jmp eax

.align 16
gdtr32:
    .word gdt32_end - gdt32_begin - 1
patch_addr:
    .long gdt32_begin - KERNBASE

.align 16
gdt32_begin:
    .long 0x00000000, 0x00000000      # Null descriptor
    .long 0x0000ffff, 0x00cf9a00      # Code segment
    .long 0x0000ffff, 0x00cf9200      # Data segment
gdt32_end:
