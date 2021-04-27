// 13519214 - Standard function

#include "std-header/std_fileio.h"
#include "std-header/boolean.h"
#include "kernel-header/config.h"

// ---------------- File I/O ----------------
void write(char *buffer, char *path, int *returncode, char parentIndex) {
    int AX = parentIndex << 8;
    AX |= 0x05;
    interrupt(0x21, AX, buffer, path, returncode);
}

void read(char *buffer, char *path, int *returncode, char parentIndex) {
    int AX = parentIndex << 8;
    AX |= 0x04;
    interrupt(0x21, AX, buffer, path, returncode);
}

void directSectorWrite(char *buffer, int sector) {
    interrupt(0x21, 0x03, buffer, sector, 0);
}

void directSectorRead(char *buffer, int sector) {
    interrupt(0x21, 0x02, buffer, sector, 0);
}

// ---------------- Misc ----------------
void memcpy(char *dest, char *src, int bytes) {
    int i = 0;
    while (i < bytes) {
        dest[i] = src[i];
        i++;
    }
}

void exec(char *filename, int segment, char parentIndex) {
    int AX = parentIndex << 8;
    int ret_code;
    AX |= 0x06;
    interrupt(0x21, AX, filename, segment, &ret_code);
    if (ret_code == -1) {
        print(filename, BIOS_LIGHT_RED);
        print(" not found!\n", BIOS_LIGHT_RED);
    }
}

void remove(char *filename, int *returncode, char parentIndex) {
    interrupt(0x21, 0x07, filename, returncode, parentIndex);
}
