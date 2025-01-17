#pragma once

#define KERNBASE 0xf0000000
#define PGSIZE   0x1000
#define PHYSTOP  0x8000000

#define PGROUNDUP(sz)  (((sz)+PGSIZE-1) & ~(PGSIZE-1))
#define V2P(a) (((uintptr_t) (a)) - KERNBASE)
