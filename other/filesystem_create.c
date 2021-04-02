// 13519214 - Simple binary file writer
// v1.1 - Adding 0xFF to file system itself
// v1.2 - Splitting loop and adding flag system, check kernel.h config
// v1.3 - More configurable & modular
#include <stdio.h>
#include <stdlib.h>

// Configuration
#define KERNEL_MAX 32
#define FILES_ENTRY "\xFF\xFE\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // Entire 16 bytes of empty files entry

#define FILESYSTEM_LOCATION 0x100
#define FILESYSTEM_SIZE 4

#define FILLED_MAP_ENTRY "\xFF"
#define EMPTY_MAP_ENTRY "\x00"

#define EMPTY_SECTORS_ENTRY "\x00"

#define SECTOR_SIZE 512

int main(int argc, char const *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage : fscreate <map path> <file path> <sector path>\n");
        exit(1);
    }
    FILE *map = fopen(argv[1], "wb");
    FILE *file = fopen(argv[2], "wb");
    FILE *sector = fopen(argv[3], "wb");

    for (int i = 0; i < SECTOR_SIZE; i++) {
        if (i < KERNEL_MAX || (i >= FILESYSTEM_LOCATION && i < FILESYSTEM_LOCATION + FILESYSTEM_SIZE))
            fwrite(FILLED_MAP_ENTRY, 1, 1, map);
        else
            fwrite(EMPTY_MAP_ENTRY, 1, 1, map);
    }

    for (int i = 0; i < (SECTOR_SIZE/0x10 * 2); i++) // 2 files filesystem, 1 entry consist 0x10 bytes
        fwrite(FILES_ENTRY, 0x10, 1, file);

    for (int i = 0; i < SECTOR_SIZE; i++)
        fwrite(EMPTY_SECTORS_ENTRY, 1, 1, sector);


    fclose(map);
    fclose(file);
    fclose(sector);
    return 0;
}
