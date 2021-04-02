// 13519214 - File inserter
// TODO : Extra, Cleanup (?), also explicit type casting for pleasing mr. gcc
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/std-header/boolean.h"

#define SECTOR_SIZE 512
#define TARGET_SECTOR_SIZE 2880 // dd ... count=2880
#define MAX_FILE_SIZE 8192 // 512*16 sectors

// Actually ripping from kernel.h
// TODO : Extra, split filesystem configuration and include
#define FILE_SECTOR_SIZE 0x10 // 16 sectors (8192 bytes) for 1 file entry
#define MAP_SECTOR 0x100
#define FILES_SECTOR 0x101
#define SECTORS_SECTOR 0x103

// Predefined values in map filesystem
#define EMPTY_MAP_ENTRY 0x00 // For empty entry
#define FILLED_MAP_ENTRY 0xFF // If sector are filled

// Flags in files filesystem
#define ROOT_PARENT_FOLDER 0xFF // Flag for "P" byte
#define EMPTY_FILES_ENTRY 0xFE // Flag for "S" byte
#define FOLDER_ENTRY 0xFF // Flag for "S" byte
#define PARENT_BYTE_OFFSET 0x0 // "P" byte, parent folder index
#define ENTRY_BYTE_OFFSET 0x1 // "S" byte, entry index at sectors filesystem
#define PATHNAME_BYTE_OFFSET 0x2 // 14 bytes, filled with pathnames

// Predefined values in sectors filesystem
#define EMPTY_SECTORS_ENTRY 0x00 // For empty entry


void clear(unsigned char *string, int length) {
    for (int i = 0; i < length; i++)
        string[i] = '\0';
}

int modstrlen(unsigned char *string) {
    int i = 0;
    while (string[i] != '\0' && i < MAX_FILE_SIZE)
        i++;
    return i;
}

void memcpybounded(unsigned char *dest, const unsigned char *src, int bytes) {
    int i = 0;
    while (i < bytes) {
        dest[i] = src[i];
        i++;
    }
}

bool modstrcmp(const unsigned char *s1, const unsigned char *s2) {
    int i = 0;
    if (modstrlen(s1) == modstrlen(s2)) {
        // If string length matches, check every char
        while (s1[i] != '\0') {
            if (s1[i] != s2[i])
                return true;
            i++;
        }

        // If both string matches
        return false;
    }

    return true;
}

void rawstrcpybounded(unsigned char *dest, const char *src, int n) {
    int i = 0;
    while (src[i] != '\0' && i < n) {
        dest[i] = src[i];
        i++;
    }
}

void strcpybounded(unsigned char *dest, const char *src, int n) {
    int i = 0;
    while (src[i] != '\0' && i < n) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

int main(int argc, char const *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage : loadFile <target> <file>\n");
        exit(1);
    }
    // Load entire file and save to buffer
    FILE *target = fopen(argv[1], "rb");
    FILE *input = fopen(argv[2], "rb");
    if (target == NULL || input == NULL) {
        if (target == NULL)
            fprintf(stderr, "Error : File target <%s> not found\n", argv[1]);
        else
            fclose(target);

        if (input == NULL)
            fprintf(stderr, "Error : File input <%s> not found\n", argv[2]);
        else
            fclose(input);
        exit(1);
    }

    unsigned char targetbuffer[TARGET_SECTOR_SIZE][SECTOR_SIZE]; // Char as bytes
    unsigned char inputbuffer[MAX_FILE_SIZE];

    for (int i = 0; i < TARGET_SECTOR_SIZE; i++)
        fread(targetbuffer[i], SECTOR_SIZE, 1, target);

    clear(inputbuffer, MAX_FILE_SIZE);
    fread(inputbuffer, MAX_FILE_SIZE, 1, input);
    int fileinput_bytesize = ftell(input);

    fclose(target);
    fclose(input);

    // Write new file with samename
    target = fopen(argv[1], "wb");

    // writeFile() written in an unoptimized code
    bool write_success = false;
    bool exist_empty_file_entry = false, valid_filename = true;
    int f_entry_sector_idx, f_entry_idx;
    if (strlen(argv[2]) <= 14) {
        for (int i = FILES_SECTOR; i < FILES_SECTOR + 2; i++) {
            for (int j = 0; j < SECTOR_SIZE && !exist_empty_file_entry; j += 0x10) {
                if ((targetbuffer[i][ENTRY_BYTE_OFFSET+j] == EMPTY_FILES_ENTRY) && !exist_empty_file_entry) {
                    exist_empty_file_entry = true;
                    f_entry_sector_idx = i;
                    f_entry_idx = j;
                }
                if (!modstrcmp(argv[2], targetbuffer[i]+PATHNAME_BYTE_OFFSET+j))
                    valid_filename = false;
            }
        }
    }

    bool is_enough_sector_in_map = false;
    if (exist_empty_file_entry && valid_filename) {
        int free_bytes = 0;
        for (int i = 0; i < (SECTOR_SIZE >> 1); i++)
            if (targetbuffer[MAP_SECTOR][i] == EMPTY_MAP_ENTRY)
                free_bytes += SECTOR_SIZE;
        if (free_bytes >= fileinput_bytesize)
            is_enough_sector_in_map = true;
    }

    bool is_empty_sectors_entry_exist = false;
    int sectors_entry_idx = 0;
    if (is_enough_sector_in_map) {
        for (int i = 0; i < SECTOR_SIZE; i += 0x10) {
            bool is_this_entry_empty = true;
            for (int j = 0; j < 0x10; j++)
                if (targetbuffer[SECTORS_SECTOR][i+j] != EMPTY_SECTORS_ENTRY)
                    is_this_entry_empty = false;
            if (is_this_entry_empty && !is_empty_sectors_entry_exist) {
                is_empty_sectors_entry_exist = true;
                sectors_entry_idx = i >> 4; // Divide by 16
            }
        }
    }

    int sector_used[16], s_u_length = 0;
    for (int i = 0; i < 16; i++)
        sector_used[i] = -1;
    if (is_empty_sectors_entry_exist) {
        int byte_left = fileinput_bytesize;
        int map_idx = 0, current_file_segment = 0, sector_idx = 0;
        while (byte_left > 0 && map_idx < (SECTOR_SIZE >> 1)) {
            if (targetbuffer[MAP_SECTOR][map_idx] == EMPTY_MAP_ENTRY) {
                targetbuffer[MAP_SECTOR][map_idx] = FILLED_MAP_ENTRY;
                targetbuffer[SECTORS_SECTOR][sectors_entry_idx*0x10+sector_idx] = map_idx;
                sector_idx++;
                memcpybounded(targetbuffer[map_idx], inputbuffer+current_file_segment*SECTOR_SIZE, SECTOR_SIZE);
                current_file_segment++;
                sector_used[s_u_length++] = map_idx;
                byte_left -= SECTOR_SIZE;
            }
            map_idx++;
        }

        targetbuffer[f_entry_sector_idx][PARENT_BYTE_OFFSET+f_entry_idx] = ROOT_PARENT_FOLDER;
        targetbuffer[f_entry_sector_idx][ENTRY_BYTE_OFFSET+f_entry_idx] = sectors_entry_idx;
        rawstrcpybounded(targetbuffer[f_entry_sector_idx]+PATHNAME_BYTE_OFFSET+f_entry_idx, argv[2], 14);
        write_success = true;
    }




    // Overwrite old file
    for (int i = 0; i < TARGET_SECTOR_SIZE; i++)
        fwrite(targetbuffer[i], SECTOR_SIZE, 1, target);

    if (write_success) {
        printf("File write success\n");
        printf("<%s> : %d bytes\n", argv[2], fileinput_bytesize);
        printf("Stats\n");
        printf("Files - Entry sector : 0x%x\n", f_entry_sector_idx);
        printf("Files - Entry index  : 0x%x\n", f_entry_idx);
        printf("Files - Entry offset : 0x%x\n", f_entry_sector_idx*SECTOR_SIZE + 0x10*f_entry_idx);
        printf("Files - P byte       : 0x%x\n", ROOT_PARENT_FOLDER);
        printf("Files - S byte       : 0x%x\n", sectors_entry_idx);
        printf("Files - Pathname     : %s\n\n", argv[2]);


        printf("Sectors - List sector used\n");
        for (int i = 0; i < s_u_length; i++)
            printf("Sector 0x%2x,  Offset 0x%2x\n", sector_used[i], sector_used[i]*0x200);
    }
    else {
        fprintf(stderr, "Failed to insert file\n");
        if (!valid_filename)
            fprintf(stderr, "Error : Filename <%s> exists in entry\n", argv[2]);
        else if (!exist_empty_file_entry)
            fprintf(stderr, "Error : Files filesystem full\n");
        else if (!is_enough_sector_in_map)
            fprintf(stderr, "Error : No empty sector found in map\n");
        else if (!is_empty_sectors_entry_exist)
            fprintf(stderr, "Error : No empty entry found in sectors filesystem\n");
    }

    fclose(target);
    return 0;
}
