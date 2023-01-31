#include "fs.h"
#include "../lib/string.h"
#include "../drivers/ata.h"
#include "../console.h"

int stat(const char* name, struct stat *buf) {
    panic("stat not implemented");
}

int read_file(const char* name, void* buf, uint32_t bufsize) {
    panic("read_file not implemented");
}
