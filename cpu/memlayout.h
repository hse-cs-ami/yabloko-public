#pragma once

#define KERNBASE 0x80000000
#define PGSIZE   0x1000
#define PHYSTOP  0x8000000

#define PGROUNDUP(sz)  (((sz)+PGSIZE-1) & ~(PGSIZE-1))
#define PGROUNDDOWN(a) (((a)) & ~((uintptr_t)(PGSIZE-1)))

#define NPDENTRIES      1024    // # directory entries per page directory
#define NPTENTRIES      1024    // # PTEs per page table

#define PTXSHIFT        12      // offset of PTX in a linear address
#define PDXSHIFT        22      // offset of PDX in a linear address

#define PXMASK          0x3FF

#ifndef __ASSEMBLER__
#include <stdint.h>
#define V2P(a) (((uintptr_t) (a)) - KERNBASE)
#define P2V(a) ((void *)(((uintptr_t) (a)) + KERNBASE))

// page directory index
#define PDX(va)         (((uintptr_t)(va) >> PDXSHIFT) & PXMASK)

// page table index
#define PTX(va)         (((uintptr_t)(va) >> PTXSHIFT) & PXMASK)

// Address in page table or page directory entry
#define PTE_ADDR(pte)   ((uintptr_t)(pte) & ~0xFFF)
#endif

// Page table/directory entry flags.
#define PTE_P           0x001   // Present
#define PTE_W           0x002   // Writeable
#define PTE_U           0x004   // User
#define PTE_PWT         0x008   // Write-Through
#define PTE_PCD         0x010   // Cache-Disable
#define PTE_A           0x020   // Accessed
#define PTE_D           0x040   // Dirty
#define PTE_PS          0x080   // Page Size
