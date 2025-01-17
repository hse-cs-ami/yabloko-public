// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include <stdint.h>
#include "mem.h"
#include "console.h"
#include "cpu/memlayout.h"
#include "cpu/x86.h"

struct run {
  struct run *next;
};

struct {
  struct run *freelist;
} kmem;

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uintptr_t)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}

void*
memset(void *dst, unsigned c, uint64_t n)
{
  if ((uintptr_t)dst%4 == 0 && n%4 == 0){
    c &= 0xFF;
    stosl(dst, (c<<24)|(c<<16)|(c<<8)|c, n/4);
  } else
    stosb(dst, c, n);
  return dst;
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *v)
{
  struct run *r;

  if((uintptr_t)v % PGSIZE || V2P(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  r = v;
  r->next = kmem.freelist;
  kmem.freelist = r;
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void* kalloc(void)
{
  struct run *r;

  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  return (char*)r;
}
