#include "kernel-header/config.h"
#include "std-header/std_stringio.h"
#include "shell-header/shell_common.h"
#include "basic-header/std_opr.h"
#include "std-header/boolean.h"

// Available parameter
// -s , create softlink, currently cd not supporting softlink

void ln(char *dirtable, char current_dir_index, char flags, char *target, char *linkname);

int main() {
    char directory_table[FILES_SECTOR_SIZE*SECTOR_SIZE];
    char shell_cache[SECTOR_SIZE];
    char arg_vector[ARGC_MAX][ARG_LENGTH];
    char argc = 0;
    int current_dir_index;

    clear(shell_cache, SECTOR_SIZE);
    getDirectoryTable(directory_table);
    getShellCache(shell_cache);

    clear(arg_vector, ARGC_MAX*ARG_LENGTH);
    memcpy(arg_vector[0], shell_cache+ARGV_OFFSET, ARG_LENGTH);
    memcpy(arg_vector[1], shell_cache+ARGV_2_OFFSET, ARG_LENGTH);
    memcpy(arg_vector[2], shell_cache+ARGV_3_OFFSET, ARG_LENGTH);
    argc = shell_cache[ARGC_OFFSET];
    current_dir_index = shell_cache[CURRENT_DIR_CACHE_OFFSET];

    // Argument count
    if (argc == 2 && !strcmp("--help", arg_vector[0])) {
        print("Utility to create a hard link or a symbolic link (symlink)\n to an existing file or directory\n", BIOS_WHITE);
        print("Possible Usage:\n", BIOS_LIGHT_BLUE);
        print("ln [source_directory] [destination_directory]\n", BIOS_LIGHT_CYAN);
        print("ln -s [source_directory] [destination_directory]\n", BIOS_LIGHT_CYAN);
    }
    else if (argc >= 3) {
        if (!strcmp("-s", arg_vector[0])) {
            print("ln: softlink mode\n", BIOS_WHITE);
            ln(directory_table, current_dir_index, 1, arg_vector[1], arg_vector[2]);
        }
        else
            ln(directory_table, current_dir_index, 0, arg_vector[0], arg_vector[1]);
    }
    else
        print("Usage : ln [-s] <source> <destination>\n", BIOS_WHITE);

    setShellCache(shell_cache);
    shellReturn();
}

void ln(char *dirtable, char current_dir_index, char flags, char *target, char *linkname) {
    // Technically just "copy" of previous implementation of ln
    // char as string / char
    char filename_buffer[16];
    char source_directory_name[16];
    char copied_directory_name[16];
    // char as 1 byte integer
    char file_read[FILE_SIZE_MAXIMUM];
    char target_entry_byte = 0;
    char source_dir_idx;
    char copied_dir_idx;
    char link_status;
    char parent_entry_byte;
    char parent_files_index;
    int returncode_src = 0;
    int returncode_cpy = 0;
    bool is_write_success = false, valid_filename = true;
    bool f_target_found = false, empty_entry_found = false;
    bool is_found_parent = false;
    bool is_found_empty = false;
    bool is_hardlink_file = true;
    int i = 0, j = 0;
    int f_entry_idx = 0;
    int f_entry_sector_idx = 0;
    int last_slash_index;


    // Relative pathing
    clear(source_directory_name, 16);
    clear(copied_directory_name, 16);
    // Split target / source
    last_slash_index = getLastMatchedCharIdx(CHAR_SLASH, target);
    // FIXME : Extra, unsafe getlast
    if (last_slash_index != -1) {
        // Split argument to path and filename
        // Get path
        strcpybounded(source_directory_name, target, last_slash_index);
        source_dir_idx = directoryEvaluator(dirtable, source_directory_name, &returncode_src, current_dir_index);

        // Get filename
        strcpybounded(source_directory_name, target+last_slash_index+1, ARG_LENGTH-last_slash_index-1);
    }
    else {
        // Cut slash
        strcpybounded(source_directory_name, target, ARG_LENGTH);

        source_dir_idx = current_dir_index;
        returncode_src = 0;
    }

    // Split destination / copied
    last_slash_index = getLastMatchedCharIdx(CHAR_SLASH, linkname);
    // FIXME : Extra, unsafe getlast
    if (last_slash_index != -1) {
        // Split argument to path and filename
        // Get path
        strcpybounded(copied_directory_name, linkname, last_slash_index);
        copied_dir_idx = directoryEvaluator(dirtable, copied_directory_name, &returncode_cpy, current_dir_index);

        // Get filename
        strcpybounded(copied_directory_name, linkname+last_slash_index+1, ARG_LENGTH-last_slash_index-1);
    }
    else {
        // Cut slash
        strcpybounded(copied_directory_name, linkname, ARG_LENGTH);

        copied_dir_idx = current_dir_index;
        returncode_src = 0;
    }


    // Copying file if path evaluation success
    if (returncode_src == -1) {
        print("ln: ", BIOS_GRAY);
        print(target, BIOS_GRAY);
        print(": target not found\n", BIOS_GRAY);
    }
    else if (returncode_cpy == -1) {
        print("ln: ", BIOS_GRAY);
        print(copied_directory_name, BIOS_GRAY);
        print(": path not found\n", BIOS_GRAY);
    }
    else {
        clear(file_read, FILE_SIZE_MAXIMUM);
        read(file_read, copied_directory_name, &returncode_cpy, copied_dir_idx);
        clear(file_read, FILE_SIZE_MAXIMUM);
        read(file_read, source_directory_name, &returncode_src, source_dir_idx);
        if (returncode_src == 0 && returncode_cpy == -1) {
            if (flags == 1)
                link_status = SOFTLINK_ENTRY;
            else {
                if (file_read[0] == NULL) {
                    print("ln: ", BIOS_GRAY);
                    print(source_directory_name, BIOS_GRAY);
                    print(": cannot hardlink folder\n", BIOS_GRAY);
                    is_hardlink_file = false;
                    returncode_src = -1;
                }
                else
                    link_status = HARDLINK_ENTRY;
            }

            // Find parent S byte
            i = 0;
            while (i < FILES_ENTRY_COUNT && !is_found_parent) {
                clear(filename_buffer, 16);
                strcpybounded(filename_buffer, dirtable+FILES_ENTRY_SIZE*i+PATHNAME_BYTE_OFFSET, 14);
                if (dirtable[i*FILES_ENTRY_SIZE + PARENT_BYTE_OFFSET] == source_dir_idx &&
                    !strcmp(source_directory_name, filename_buffer)) {
                    is_found_parent = true;
                    parent_entry_byte = dirtable[i*FILES_ENTRY_SIZE + ENTRY_BYTE_OFFSET];
                    parent_files_index = i;
                }
                i++;
            }

            // Find empty files entry
            i = 0;
            while (i < FILES_ENTRY_COUNT && !is_found_empty && is_found_parent) {
                clear(filename_buffer, 16);
                strcpybounded(filename_buffer, dirtable+FILES_ENTRY_SIZE*i+PATHNAME_BYTE_OFFSET, 14);
                if (dirtable[i*FILES_ENTRY_SIZE + PARENT_BYTE_OFFSET] == ROOT_PARENT_FOLDER &&
                    dirtable[i*FILES_ENTRY_SIZE + ENTRY_BYTE_OFFSET] == EMPTY_FILES_ENTRY) {
                        is_found_empty = true;
                        // Put P and filename
                        dirtable[i*FILES_ENTRY_SIZE+PARENT_BYTE_OFFSET] = copied_dir_idx;
                        memcpy(dirtable+FILES_ENTRY_SIZE*i+PATHNAME_BYTE_OFFSET, copied_directory_name, 14);
                        dirtable[i*FILES_ENTRY_SIZE+LINK_BYTE_OFFSET] = link_status;
                        // Put S byte
                        if (flags == 0)
                            dirtable[i*FILES_ENTRY_SIZE+ENTRY_BYTE_OFFSET] = parent_entry_byte;
                        else
                            dirtable[i*FILES_ENTRY_SIZE+ENTRY_BYTE_OFFSET] = parent_files_index;
                    }
                i++;
            }


            if (!is_found_parent) {
                print("ln: searching parent ", BIOS_WHITE);
                print(target, BIOS_WHITE);
                print(" error\n", BIOS_WHITE);
                returncode_cpy = -1;
            }
            else if (!is_found_empty) {
                print("ln: searching empty space for ", BIOS_WHITE);
                print(linkname, BIOS_WHITE);
                print(" error\n", BIOS_WHITE);
                returncode_cpy = -1;
            }
            else if (is_hardlink_file) {
                // Updating directory table
                directSectorWrite(dirtable, FILES_SECTOR);
                directSectorWrite(dirtable+SECTOR_SIZE, FILES_SECTOR+1);
            }

        }
        else if (returncode_src == -1) {
            print("ln: error: ", BIOS_WHITE);
            print(target, BIOS_WHITE);
            print(" not found\n", BIOS_WHITE);
            returncode_src = -1;
        }
        else if (returncode_cpy == 0) {
            print("ln: error: ", BIOS_WHITE);
            print(linkname, BIOS_WHITE);
            print(" exist\n", BIOS_WHITE);
            returncode_cpy = 0;
        }

        if (returncode_src == 0 && returncode_cpy == -1) {
            print(linkname, BIOS_GRAY);
            print(": link created\n", BIOS_GRAY);
        }
        else
            print("ln: file writing error\n", BIOS_GRAY);
    }
}
