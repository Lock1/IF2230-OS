#include "kernel-header/config.h"
#include "std-header/std_stringio.h"
#include "std-header/boolean.h"
#include "shell-header/shell_common.h"
#include "basic-header/std_opr.h"

char cd(char *dirtable, char *dirstr, char current_dir);

int main() {
    char directory_table[FILES_SECTOR_SIZE*SECTOR_SIZE];
    char shell_cache[SECTOR_SIZE];
    char dirstr[BUFFER_SIZE];
    char new_index;
    int argc;

    clear(shell_cache, SECTOR_SIZE);
    getDirectoryTable(directory_table);
    getShellCache(shell_cache);

    clear(dirstr, BUFFER_SIZE);
    memcpy(dirstr, shell_cache+ARGV_OFFSET, ARG_LENGTH);
    argc = shell_cache[ARGC_OFFSET];

    if (argc == 2) {
        if (!strcmp("--help", dirstr)) {
            print("Utility to change working directory\n", BIOS_WHITE);
            print("Possible Usage:\n", BIOS_LIGHT_BLUE);
            print("cd [directory_name]\n", BIOS_LIGHT_CYAN);
            print("cd ..\n", BIOS_LIGHT_CYAN);
        }
        else {
            new_index = cd(directory_table, dirstr, shell_cache[CURRENT_DIR_CACHE_OFFSET]);
            shell_cache[CURRENT_DIR_CACHE_OFFSET] = new_index;
        }
    }
    else if (argc == 1)
        shell_cache[CURRENT_DIR_CACHE_OFFSET] = ROOT_PARENT_FOLDER;
    else
        print("Usage : cd <path>\n", BIOS_WHITE);

    setShellCache(shell_cache);
    shellReturn();
}

char cd(char *dirtable, char *dirstr, char current_dir) {
    int returncode = 0;
    char new_dir_idx = directoryEvaluator(dirtable, dirstr, &returncode, current_dir);
    // Copied from other code
    int i = 0;
    bool is_found_parent = false;
    char filename_buffer[16];
    char source_dir_idx;
    char source_directory_name[16];
    int returncode_src;
    char file_read[FILE_SIZE_MAXIMUM];
    char target_parent_byte;
    char target_entry_byte;
    char link_status = -1;
    char original_softlink_entry_byte;
    int last_slash_index;

    // If success return new dir index
    if (returncode == 0) {
        return new_dir_idx;
    }
    else if (returncode == 1) {
        // Else, entry is not folder
        // Find whether file is softlink or not

        // The super smelly code
        // Relative pathing
        clear(source_directory_name, 16);
        // Split target / source
        last_slash_index = getLastMatchedCharIdx(CHAR_SLASH, dirstr);
        // FIXME : Extra, unsafe getlast
        if (last_slash_index != -1) {
            // Split argument to path and filename
            // Get path
            strcpybounded(source_directory_name, dirstr, last_slash_index);
            source_dir_idx = directoryEvaluator(dirtable, source_directory_name, &returncode_src, current_dir);

            // Get filename
            strcpybounded(source_directory_name, dirstr+last_slash_index+1, ARG_LENGTH-last_slash_index-1);
        }
        else {
            // Cut slash
            strcpybounded(source_directory_name, dirstr, ARG_LENGTH);

            source_dir_idx = current_dir;
            returncode_src = 0;
        }



        if (returncode_src == 0) {
            // Find entry in files
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
                    // Get linked strings / folder with recursion depth limit 16
                    i = 0;
                    // TODO : Extra, currently only single softlink depth supported
                    clear(filename_buffer, 16);
                    strcpybounded(filename_buffer, dirtable+target_entry_byte*FILES_ENTRY_SIZE+PATHNAME_BYTE_OFFSET, 14);
                    target_parent_byte = dirtable[target_entry_byte*FILES_ENTRY_SIZE + PARENT_BYTE_OFFSET];
                    original_softlink_entry_byte = target_entry_byte;
                    target_entry_byte = dirtable[target_entry_byte*FILES_ENTRY_SIZE + ENTRY_BYTE_OFFSET];
                    clear(file_read, FILE_SIZE_MAXIMUM);
                    read(file_read, filename_buffer, &returncode_src, target_parent_byte);
                    // Needed to compare due _mash_cache always filling empty space
                    // Implying softlink to _mash_cache is not available
                    if (!strcmp(filename_buffer, "_mash_cache"))
                        target_entry_byte = EMPTY_FILES_ENTRY;
                }

                if (target_entry_byte == FOLDER_ENTRY)
                    returncode = 0;
        }
        else {
            print("cd: path ", BIOS_WHITE);
            print(dirstr, BIOS_WHITE);
            print(" evaluation failed\n", BIOS_WHITE);
            returncode = -1;
        }

        if (returncode != 0) {
            print("cd: target type is a file\n", BIOS_WHITE);
            return current_dir;
        }
        else {
            print("cd: softlink to ", BIOS_WHITE);
            print(filename_buffer, BIOS_LIGHT_CYAN);
            print("\n", BIOS_WHITE);
            return original_softlink_entry_byte;
        }
    }
    else {
        // Else, return original dir
        print("cd: path not found\n", BIOS_WHITE);
        return current_dir;
    }
}
