#pragma once

#include <stdint.h>

enum {
	EI_NIDENT = 16,
};

typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Off;

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian

typedef struct {
	uint32_t magic;
	unsigned char e_ident[EI_NIDENT - 4];
	uint16_t      e_type;
	uint16_t      e_machine;
	uint32_t      e_version;
	Elf32_Addr    e_entry;
	Elf32_Off     e_phoff;
	Elf32_Off     e_shoff;
	uint32_t      e_flags;
	uint16_t      e_ehsize;
	uint16_t      e_phentsize;
	uint16_t      e_phnum;
	uint16_t      e_shentsize;
	uint16_t      e_shnum;
	uint16_t      e_shstrndx;
} Elf32_Ehdr;

typedef struct {
	uint32_t   p_type;
	Elf32_Off  p_offset;
	Elf32_Addr p_vaddr;
	Elf32_Addr p_paddr;
	uint32_t   p_filesz;
	uint32_t   p_memsz;
	uint32_t   p_flags;
	uint32_t   p_align;
} Elf32_Phdr;
