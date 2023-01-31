#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s mbr.bin\n", argv[0]);
        return 1;
    }
    const char* filename = argv[1];
    int fd = open(filename, O_RDWR);
    off_t length = lseek(fd, 0, SEEK_END);
    if (length > 510) {
        fprintf(stderr, "file %s is larger than 510 bytes (size: %ju)\n",
                filename, (uintmax_t)length);
        return 1;
    }
    lseek(fd, 510, SEEK_SET);
    write(fd, "\x55\xaa", 2);
    return 0;
}
