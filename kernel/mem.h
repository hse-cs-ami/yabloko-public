#pragma once

#include <stdint.h>

typedef uintptr_t pde_t;
typedef uintptr_t pte_t;

void* memset(void *dst, unsigned c, uint64_t n);

void freerange(void *vstart, void *vend);
void* kalloc(void);
void kfree(void*);

pde_t *setupkvm();
void kvmalloc();
void switchkvm();
int allocuvm(pde_t *pgdir, uintptr_t base, uintptr_t top);
void freevm(pde_t *pgdir);
