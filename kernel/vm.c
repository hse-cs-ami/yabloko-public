#include "mem.h"
#include "cpu/memlayout.h"
#include "cpu/x86.h"

pde_t *kvm;

void kvmalloc() {
    kvm = kalloc();
    memset(kvm, 0, PGSIZE);

    // Map physical memory to KERNBASE..KERNBASE+PHYSTOP
    for (uintptr_t pa = 0; pa < PHYSTOP; pa += 4 << 20) {
        uintptr_t va = KERNBASE + pa;
        kvm[PDX(va)] = pa | PTE_P | PTE_W | PTE_PS;
    }

    lcr3(V2P(kvm));
}
