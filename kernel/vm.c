#include "mem.h"
#include "cpu/memlayout.h"
#include "cpu/x86.h"
#include "console.h"

pde_t *kvm;

pde_t *setupkvm(void) {
    pde_t *kvm = kalloc();
    memset(kvm, 0, PGSIZE);

    // Map physical memory to KERNBASE..KERNBASE+PHYSTOP
    for (uintptr_t pa = 0; pa < PHYSTOP; pa += 4 << 20) {
        uintptr_t va = KERNBASE + pa;
        kvm[PDX(va)] = pa | PTE_P | PTE_W | PTE_PS;
    }
    return kvm;
}

void kvmalloc() {
    kvm = setupkvm();
    switchkvm();
}

void switchkvm(void)
{
    lcr3(V2P(kvm)); // switch to the kernel page table
}

// Return the address of the PTE in page table pgdir
// that corresponds to virtual address va.  If alloc!=0,
// create any required page table pages.
static pte_t *
walkpgdir(pde_t *pgdir, const void *va, int alloc)
{
    pde_t *pde;
    pte_t *pgtab;

    pde = &pgdir[PDX(va)];
    if (*pde & PTE_P) {
        pgtab = (pte_t *)P2V(PTE_ADDR(*pde));
    } else {
        if (!alloc || (pgtab = (pte_t *)kalloc()) == 0)
            return 0;
        // Make sure all those PTE_P bits are zero.
        memset(pgtab, 0, PGSIZE);
        // The permissions here are overly generous, but they can
        // be further restricted by the permissions in the page table
        // entries, if necessary.
        *pde = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
    }
  return &pgtab[PTX(va)];
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned.
static int
mappages(pde_t *pgdir, void *va, uintptr_t size, uintptr_t pa, int perm)
{
    char *a, *last;
    pte_t *pte;

    a = (char *)PGROUNDDOWN((uintptr_t)va);
    last = (char *)PGROUNDDOWN(((uintptr_t)va) + size - 1);
    for (;;)
    {
        if ((pte = walkpgdir(pgdir, a, 1)) == 0)
            return -1;
        if (*pte & PTE_P)
            panic("remap");
        *pte = pa | perm | PTE_P;
        if (a == last)
            break;
        a += PGSIZE;
        pa += PGSIZE;
  }
  return 0;
}

int allocuvm(pde_t *pgdir, uintptr_t base, uintptr_t top) {
    for (uintptr_t a = PGROUNDUP(base); a < top; a += PGSIZE) {
        char *pa = kalloc();
        if (pa == 0) {
            return -1;
        }
        memset(pa, 0, PGSIZE);
        if (mappages(pgdir, (void*)a, PGSIZE, V2P(pa), PTE_W | PTE_U) < 0) {
            kfree(pa);
            return -1;
        }
    }
    return 0;
}

static void freept(pte_t *pt) {
    for (int i = 0; i < NPTENTRIES; i++) {
        if (pt[i] & PTE_P) {
            kfree((char*)P2V(PTE_ADDR(pt[i])));
        }
    }
    kfree((char*)pt);
}

void freevm(pde_t *pgdir) {
    for (int i = 0; i < NPDENTRIES / 2; i++) {
        if (pgdir[i] & PTE_P) {
            freept((pte_t*)P2V(PTE_ADDR(pgdir[i])));
        }
    }
    kfree(pgdir);
}
