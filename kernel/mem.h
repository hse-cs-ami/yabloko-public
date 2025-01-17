#pragma once

#include <stdint.h>

typedef uintptr_t pde_t;
typedef uintptr_t pte_t;

void* memset(void *dst, unsigned c, uint64_t n);

void freerange(void *vstart, void *vend);
void* kalloc(void);
void kfree(char*);

void kvmalloc();