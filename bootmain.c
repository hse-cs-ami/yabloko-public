// Boot loader.
//
// Part of the boot block, along with mbr.S, which calls bootmain().
// mbr.S has put the processor into protected 32-bit mode.
// bootmain() loads an ELF kernel image from the disk starting at
// sector 3 and then jumps to the kernel entry routine.

#include "elf.h"
#include "drivers/port.h"

#define SECTSIZE  512

typedef unsigned char uchar;
typedef unsigned int uint;

void readseg(uchar*, uint, uint);

inline void stosb(uchar* addr, uchar byte, uint count) {
    asm volatile("cld; rep stosb" : "+D"(addr), "+a"(byte) : "c"(count) : "cc");
}

void
bootmain(void)
{
  Elf32_Ehdr *elf;
  Elf32_Phdr *ph, *eph;
  void (*entry)(void);
  uchar* pa;

  elf = (Elf32_Ehdr*)0x10000;  // scratch space

  // Read 1st page off disk
  readseg((uchar*)elf, 4096, 0);

  // Is this an ELF executable?
  if(elf->magic != ELF_MAGIC)
    return;  // let bootasm.S handle error

  // Load each program segment (ignores ph flags).
  ph = (Elf32_Phdr*)((uchar*)elf + elf->e_phoff);
  eph = ph + elf->e_phnum;
  for(; ph < eph; ph++) {
    pa = (uchar*)(ph->p_paddr & 0x0fffffff);
    readseg(pa, ph->p_filesz, ph->p_offset);
    if(ph->p_memsz > ph->p_filesz)
      stosb(pa + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
  }

  // Call the entry point from the ELF header.
  // Does not return!
  entry = (void(*)(void))(elf->e_entry & 0x0fffffff);
  entry();
}

void
waitdisk(void)
{
  // Wait for disk ready.
  while((port_byte_in(0x1F7) & 0xC0) != 0x40)
    ;
}

static inline void
insl(int port, void *addr, int cnt)
{
  asm volatile("cld; rep insl" :
               "=D" (addr), "=c" (cnt) :
               "d" (port), "0" (addr), "1" (cnt) :
               "memory", "cc");
}

// Read a single sector at offset into dst.
void
readsect(void *dst, uint offset)
{
  // Issue command.
  waitdisk();
  port_byte_out(0x1F2, 1);   // count = 1
  port_byte_out(0x1F3, offset);
  port_byte_out(0x1F4, offset >> 8);
  port_byte_out(0x1F5, offset >> 16);
  port_byte_out(0x1F6, (offset >> 24) | 0xE0);
  port_byte_out(0x1F7, 0x20);  // cmd 0x20 - read sectors

  // Read data.
  waitdisk();
  insl(0x1F0, dst, SECTSIZE/4);
}

// Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
// Might copy more than asked.
void
readseg(uchar* pa, uint count, uint offset)
{
  uchar* epa;

  epa = pa + count;

  // Round down to sector boundary.
  pa -= offset % SECTSIZE;

  // Translate from bytes to sectors; kernel starts at sector 3.
  offset = (offset / SECTSIZE) + 2;

  // If this is too slow, we could read lots of sectors at a time.
  // We'd write more to memory than asked, but it doesn't matter --
  // we load in increasing order.
  for(; pa < epa; pa += SECTSIZE, offset++)
    readsect(pa, offset);
}
