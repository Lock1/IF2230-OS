#include "kernel-header/config.h"
#include "std-header/std_stringio.h"
#include "shell-header/shell_common.h"
#include "basic-header/std_opr.h"
#include "std-header/boolean.h"

void cat(char *dirtable, char *filename, char target_dir);

int main() {
    char directory_table[FILES_SECTOR_SIZE*SECTOR_SIZE];
    char shell_cache[SECTOR_SIZE];
    char dirstr[BUFFER_SIZE];
    char argc = 0;
    int returncode = 0;
    char current_dir_index;

    clear(shell_cache, SECTOR_SIZE);
    getDirectoryTable(directory_table);
    getShellCache(shell_cache);

    clear(dirstr, BUFFER_SIZE);
    memcpy(dirstr, shell_cache+ARGV_OFFSET, ARG_LENGTH);
    argc = shell_cache[ARGC_OFFSET];
    current_dir_index = shell_cache[CURRENT_DIR_CACHE_OFFSET];


    // Argument count
    if (argc == 2) 
        if (!strcmp("--help", dirstr)) {
            print("Utility to view file\n", BIOS_WHITE);
            print("Possible Usage:\n", BIOS_LIGHT_BLUE);
            print("cat [file_name]\n", BIOS_LIGHT_CYAN);
        }
        else {
            cat(directory_table, dirstr, current_dir_index);
        }
    else
        print("Usage : cat <filename>\n", BIOS_WHITE);
    shellReturn();
}


void cat(char *dirtable, char *filename, char target_dir) {
    char filename_buffer[16];
    char source_directory_name[16];
    char file_read[FILE_SIZE_MAXIMUM];
    char directory_name[ARGC_MAX][ARG_LENGTH];
    char evaluator_temp[ARGC_MAX*ARG_LENGTH];
    char eval_dir = target_dir;
    char target_entry_byte = 0;
    char target_parent_byte = 0;
    char source_dir_idx;
    char link_status = -1;
    bool is_found_parent = false;
    int returncode_src = 0;
    int returncode = -1, eval_returncode = -1;
    int dirnamecount;
    int i, j, k;


    // Another splitter stealing
    clear(directory_name, ARGC_MAX*ARG_LENGTH);
    clear(evaluator_temp, ARGC_MAX*ARG_LENGTH);
    i = 0;
    j = 0;
    k = 0;
    while (filename[i] != CHAR_NULL) {
        // If found slash in commands and not within double quote mark, make new
        if (filename[i] == CHAR_SLASH && j < ARGC_MAX) {
            k = 0;
            j++;
        }
        else {
            // Only copy if char is not double quote
            // and space outside double quote
            directory_name[j][k] = filename[i];
            k++;
        }
        i++;
    }
    dirnamecount = j + 1; // Due j is between counting space between 2 args

    i = 0;
    // Note : Entry directory_name[dirnamecount - 1] is a FILENAME, not folder
    while (i < dirnamecount - 1) {
        strapp(evaluator_temp, directory_name[i]);
        strapp(evaluator_temp, "/");
        i++;
    }

    strcpybounded(source_directory_name, directory_name[dirnamecount - 1], 14); // TODO : Test

    if (dirnamecount > 1) {
        eval_dir = directoryEvaluator(dirtable, evaluator_temp, &eval_returncode, target_dir);
    }
    source_dir_idx = eval_dir;

    clear(file_read, FILE_SIZE_MAXIMUM);
    // Take last argv, use it as filename
    read(file_read, directory_name[dirnamecount-1], &returncode, eval_dir);
    if (returncode == -1) {
        print("cat: ", BIOS_GRAY);
        print(directory_name[dirnamecount-1], BIOS_GRAY);
        print(": file not found", BIOS_GRAY);
    }
    else {
        i = 0;
        while (i < FILES_ENTRY_COUNT && !is_found_parent) {
            clear(filename_buffer, 16);
            strcpybounded(filename_buffer, dirtable+FILES_ENTRY_SIZE*i+PATHNAME_BYTE_OFFSET, 14);
            if (dirtable[i*FILES_ENTRY_SIZE + PARENT_BYTE_OFFSET] == source_dir_idx &&
                !strcmp(source_directory_name, filename_buffer)) {
                is_found_parent = true;
                target_entry_byte = dirtable[i*FILES_ENTRY_SIZE+ENTRY_BYTE_OFFSET];
                link_status = dirtable[i*FILES_ENTRY_SIZE+LINK_BYTE_OFFSET];
            }
            i++;
        }

        if (link_status == SOFTLINK_ENTRY) {
            i = 0;
            // TODO : Extra, currently only single softlink depth supported
            clear(filename_buffer, 16);
            strcpybounded(filename_buffer, dirtable+target_entry_byte*FILES_ENTRY_SIZE+PATHNAME_BYTE_OFFSET, 14);
            target_parent_byte = dirtable[target_entry_byte*FILES_ENTRY_SIZE + PARENT_BYTE_OFFSET];
            target_entry_byte = dirtable[target_entry_byte*FILES_ENTRY_SIZE + ENTRY_BYTE_OFFSET];
            clear(file_read, FILE_SIZE_MAXIMUM);
            read(file_read, filename_buffer, &returncode_src, target_parent_byte);
            // Needed to compare due _mash_cache always filling empty space
            // Implying softlink to _mash_cache is not available
            if (!strcmp(filename_buffer, "_mash_cache"))
                target_entry_byte = EMPTY_FILES_ENTRY;
        }

        if (returncode_src == 0 && target_entry_byte != EMPTY_FILES_ENTRY) {
            if (file_read[0] == NULL) {
                print("cat: ", BIOS_WHITE);
                print(directory_name[dirnamecount-1], BIOS_WHITE);
                print(" is a folder", BIOS_WHITE);
            }
            else {
                i = 0;
                while (i < FILE_SIZE_MAXIMUM && file_read[i] != CHAR_NULL) {
                    if (file_read[i] == CHAR_CARRIAGE_RETURN)
                        file_read[i] = CHAR_SPACE;
                    i++;
                }
                print(file_read, BIOS_GRAY);
            }
        }
        else {
            print("cat: ", BIOS_WHITE);
            print(source_directory_name, BIOS_WHITE);
            print(" softlink broken\n", BIOS_WHITE);
        }
    }
    print("\n\n", BIOS_GRAY);
}
