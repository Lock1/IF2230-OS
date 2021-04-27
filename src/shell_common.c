#include "kernel-header/config.h"
#include "std-header/std_fileio.h"
#include "std-header/std_stringio.h"
#include "shell-header/shell_common.h"
#include "std-header/boolean.h"

extern int interrupt(int number, int AX, int BX, int CX, int DX);

void getDirectoryTable(char *buffer) {
    // WARNING : Naive implementation
    directSectorRead(buffer, FILES_SECTOR);
    directSectorRead(buffer + SECTOR_SIZE, FILES_SECTOR + 1);
}

void shellReturn() {
    int AX = BIN_PARENT_FOLDER << 8;
    int ret_code;
    AX |= 0x06;
    interrupt(0x21, AX, "mash", 0x2000, &ret_code);
}

void getShellCache(char *buffer) {
    int returncode;
    read(buffer, "_mash_cache\0\0\0", &returncode, ROOT_PARENT_FOLDER);
}

void setShellCache(char *buffer) {
    int returncode;
    remove("_mash_cache", &returncode, ROOT_PARENT_FOLDER);
    write(buffer, "_mash_cache\0\0\0", &returncode, ROOT_PARENT_FOLDER);
}

bool isBinaryFileMagicNumber(char *buffer) {
    return !forcestrcmp(EXECUTABLE_SIGNATURE, buffer);
}



char directoryEvaluator(char *dirtable, char *dirstr, int *returncode, char current_dir) {
    // FIXME : Extra, will have problem with file & folder with same name on same directory
    // -- Evaluator return code --
    // -1 - Path not found
    // 0 - Evaluator find folder
    // 1 - Evaluator find file
    char evaluated_dir = current_dir;
    char parent_byte_buffer = -1;
    char entry_byte_buffer = -1;
    char directory_name[ARGC_MAX][ARG_LENGTH];
    char filename_buffer[16];
    int i, j, k, dirnamecount;
    bool is_valid_args = true, is_folder_found = false;
    bool is_type_is_folder = true;
    clear(directory_name, ARGC_MAX*ARG_LENGTH);

    // TODO : Extra, maybe std -> strsplit()
    // Arguments splitting -> From argv in shell(), with some modification
    i = 0;
    j = 0;
    k = 0;
    while (dirstr[i] != CHAR_NULL) {
        // If found slash in commands and not within double quote mark, make new
        if (dirstr[i] == CHAR_SLASH && j < ARGC_MAX) {
            k = 0;
            j++;
        }
        else {
            // Only copy if char is not double quote
            // and space outside double quote
            directory_name[j][k] = dirstr[i];
            k++;
        }
        i++;
    }
    dirnamecount = j + 1; // Due j is between counting space between 2 args

    // Deleting last slash (ex. mnt/a/b/ -> argv entries = {mnt, a, b, ""} to argv = {mnt, a, b})
    if (!strcmp(directory_name[dirnamecount-1], ""))
        dirnamecount--;


    // Parsing priority :
    // 1. If found "." -> change evaluated dir to current dir, will ignoring previous evaluation
    // 2. If found ".." -> move to parent folder
    // 3. If found foldername -> search foldername in evaluated dir
    i = 0;
    while (i < dirnamecount && is_valid_args) {
        if (!strcmp(directory_name[i], ".")) {
            evaluated_dir = current_dir;
        }
        else if (!strcmp(directory_name[i], "..")) {
            // If evaluated dir is NOT in between files entry count and 0 (or valid files index), do nothing
            // (Root flag by default is on 0xFF which is by default not in range)
            // else, change evaluated dir to parent evaluated dir
            if (0 <= evaluated_dir && evaluated_dir < FILES_ENTRY_COUNT)
                evaluated_dir = dirtable[evaluated_dir*FILES_ENTRY_SIZE+PARENT_BYTE_OFFSET];
        }
        else {
            // If string matching not found, break loop return -1 as failed evaluation
            j = 0;
            is_folder_found = false;
            while (j < FILES_ENTRY_COUNT && !is_folder_found) {
                clear(filename_buffer, 16);
                strcpybounded(filename_buffer, dirtable+j*FILES_ENTRY_SIZE+PATHNAME_BYTE_OFFSET, 14);
                // If within same parent folder and pathname match, change evaluated_dir
                parent_byte_buffer = dirtable[j*FILES_ENTRY_SIZE+PARENT_BYTE_OFFSET];
                entry_byte_buffer = dirtable[j*FILES_ENTRY_SIZE+ENTRY_BYTE_OFFSET];
                if (!strcmp(directory_name[i], filename_buffer) && parent_byte_buffer == evaluated_dir) {
                    is_folder_found = true;
                    is_type_is_folder = (entry_byte_buffer == FOLDER_ENTRY);
                    evaluated_dir = j; // NOTE : j represent files entry index
                }
                j++;
            }

            if (!is_folder_found)
                is_valid_args = false;
        }
        i++;
    }

    if (!is_valid_args)
        *returncode = -1;
    else if (is_type_is_folder)
        *returncode = 0;
    else
        *returncode = 1;

    return evaluated_dir;
}
