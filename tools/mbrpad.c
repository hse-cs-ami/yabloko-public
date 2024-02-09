#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>

enum {
    MBR_END = 510,
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s mbr.bin fs.img\n", argv[0]);
        return 1;
    }
    const char* filename = argv[1];
    const char* fsimg_path = argv[2];
    int fd = open(filename, O_RDWR);
    off_t length = lseek(fd, 0, SEEK_END);
    if (length > MBR_END) {
        fprintf(stderr, "file %s is larger than %d bytes (size: %llu)\n",
                filename, MBR_END, (unsigned long long)length);
        return 1;
    }
    lseek(fd, MBR_END, SEEK_SET);
    write(fd, "\x55\xaa", 2);

    int fsimg = open(fsimg_path, O_RDONLY);
    if (fsimg < 0) {
        perror(fsimg_path);
        return 1;
    }
    char buf[1024];
    ssize_t bytes;
    while ((bytes = read(fsimg, buf, sizeof(buf))) > 0) {
        write(fd, buf, bytes);
    }
    return 0;
}
