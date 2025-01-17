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
    // zero out PD at 0x1000
    xor eax, eax
    mov ecx, 1024
    rep stosd

    // Enable 4 MiB pages
    mov eax, cr4
    or eax, 0x10        // Set the PSE bit (bit 4)
    mov cr4, eax

    // Identity map low 4 MiB
    mov dword ptr [0x1000], 0 | PTE_P | PTE_W | PTE_PS

    // KERNBASE = 0x8000_0000
    // Same mapping for the first 4 MiB after KERNBASE
    mov dword ptr [0x1000 + ((KERNBASE >> 22) * 4)], 0 | PTE_P | PTE_W | PTE_PS

    // Load physical address of PD into CR3
    mov edi, 0x1000
    mov cr3, edi

    // Enable paging
    mov eax, cr0
    or eax, 1 << 31      // Set the PG bit
    mov cr0, eax

    // jump to the high half
    add esp, KERNBASE
    lea eax, kmain
    jmp eax
