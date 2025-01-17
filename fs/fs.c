#include "fs.h"
#include "../lib/string.h"
#include "../drivers/ata.h"

enum {
    fs_start = 1, // sector where the FS starts
};

int stat(const char* name, struct stat *buf) {
    struct dir dir;
    read_sectors_ATA_PIO(&dir, fs_start, 1);
    for (int i = 0; i < ents_in_dir; ++i) {
        struct dirent *e = &dir.entries[i];
        if (!strncmp(e->name, name, sizeof(e->name))) {
            buf->size = e->size_bytes;
            buf->start_sector = e->offset_sectors;
            return 0;
        }
    }
    return -1;
}

/* Find file by name and read it into buffer with size bufsize.
 * At most (bufsize & -512) bytes will be read.
 * Return number of bytes read or -1 on failure. */
int read_file(const struct stat *statbuf, void* buf, uint32_t bufsize) {
    uint32_t sector = fs_start + statbuf->start_sector;
    uint32_t bytes_read = 0;
    uint32_t file_sectors = (statbuf->size + sector_size - 1) / sector_size;
    while (bufsize >= sector_size && file_sectors > 0) {
        read_sectors_ATA_PIO(buf, sector, 1);
        sector++;
        file_sectors--;
        bufsize -= sector_size;
        buf += sector_size;
        bytes_read += sector_size;
    }
    return bytes_read < statbuf->size ? bytes_read : statbuf->size;
}
