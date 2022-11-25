#include "../fs/fs.h"

#include <stdio.h>
#include <string.h>

char* basename(char* path) {
    char* c = strrchr(path, '/');
    if (c && *c) {
        return c + 1;
    }
    return path;
}

int main(int argc, char* argv[]) {
    char sector[sector_size];
    struct dir dir = {{0}};

    if (argc < 3) {
        fprintf(stderr, "Usage: %s OUT.FS KERNEL.BIN [FILES...]\n", argv[0]);
        return 1;
    }

    FILE* image = fopen(argv[1], "wb");
    if (!image) {
        perror(argv[1]);
        return 1;
    }

    if (fwrite(&dir, sizeof(dir), 1, image) < 1) {
        perror("fwrite");
        return 1;
    }
    uint32_t sector_offset = 1;

    for (int i = 2; i < argc; ++i) {
        char* name = argv[i];
        struct dirent *dirent = &dir.entries[i-2];
        dirent->offset_sectors = sector_offset;
        dirent->size_bytes = 0;

        FILE* file = fopen(name, "rb");
        if (!file) {
            perror(name);
            return 1;
        }

        size_t read_size;
        while ((read_size = fread(sector, 1, sizeof(sector), file))) {
            if (fwrite(sector, 1, sizeof(sector), image) != sizeof(sector)) {
                perror(name);
                return 1;
            }
            sector_offset++;
            dirent->size_bytes += read_size;
        }

        if (fclose(file)) {
            perror(name);
            return 1;
        }

        dirent->reserved = 0;
        dirent->name[sizeof(dirent->name) - 1] = '\0';
        strncpy(dirent->name, basename(name), sizeof(dirent->name) - 1);
    }

    fseek(image, 0, SEEK_SET);
    if (fwrite(&dir, sizeof(dir), 1, image) < 1) {
        perror("fwrite");
        return 1;
    }

    if (fclose(image)) {
        perror(argv[0]);
        return 1;
    }

    return 0;
}
