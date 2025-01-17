#pragma once

#include <stdint.h>

/* Directory structure:

         32-byte entries
┌───────────────────────────────┐
│Reserved                       │
│char[32]                       │
├──────┬──────┬────────┬────────┤
│Offset│Size  │Reserved│Name    │
│uint32│uint32│uint32  │char[20]│
├──────┼──────┼────────┼────────┤
│ ...  │      │        │        │

Offset is in sectors (zero-based),
size is in bytes, name is 0-terminated.

*/

enum {
	sector_size = 512,
	ents_in_dir = 15,
};

struct dirent {
	uint32_t offset_sectors;
	uint32_t size_bytes;
	uint32_t reserved;
	char name[20];
};

struct dir {
	char reserved[32];
	struct dirent entries[ents_in_dir];
};

struct stat {
	uint32_t size;
	uint32_t start_sector;
	uint32_t reserved1, reserved2;
};

/* Find file by name and fill information in buf.
 * Returns zero on success, nonzero on failure. */
int stat(const char* name, struct stat *buf);

/* Find file by name and read it into buffer with size bufsize.
 * At most (bufsize & ~511) bytes will be read.
 * Return number of bytes read or -1 on failure. */
int read_file(const struct stat *statbuf, void* buf, uint32_t bufsize);
