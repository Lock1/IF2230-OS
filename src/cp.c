#include "kernel-header/config.h"
#include "std-header/std_stringio.h"
#include "shell-header/shell_common.h"
#include "basic-header/std_opr.h"
#include "std-header/boolean.h"

void cp(char *dirtable, char current_dir_index, char flags, char *target, char *linkname);

int main() {
    char directory_table[FILES_SECTOR_SIZE*SECTOR_SIZE];
    char shell_cache[SECTOR_SIZE];
    char arg_vector[ARGC_MAX][ARG_LENGTH];
    char file_read[FILE_SIZE_MAXIMUM];
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
        print("Utility to copy file\n", BIOS_WHITE);
        print("Possible Usage:\n", BIOS_LIGHT_BLUE);
        print("cp [source_directory] [target_directory]\n", BIOS_LIGHT_CYAN);
        print("cp -r [source_directory] [target_directory]\n", BIOS_LIGHT_CYAN);
    }
    else if (argc >= 3) {
        if (!strcmp("-r", arg_vector[0]))
            cp(file_read, directory_table, current_dir_index, 1, arg_vector[1], arg_vector[2]);
        else
            cp(file_read, directory_table, current_dir_index, 0, arg_vector[0], arg_vector[1]);
    }
    else
        print("Usage : cp [-r] <source> <destination>\n", BIOS_WHITE);

    setShellCache(shell_cache);
    shellReturn();
}

void file_copy(char *file_read, char *source_directory_name, char *returncode_src, char source_dir_idx,
            char *copied_directory_name, char *returncode_cpy, char copied_dir_idx, char *dirtable) {
    char filename_buffer[16];

    char target_entry_byte;
    char link_status = -1;
    char target_parent_byte;
    char original_softlink_index;

    int i = 0;
    bool empty_entry_found = false;
    bool is_found_parent = false;

    if (getKeyboardCursor(true) > 20)
        scrollScreen();

    // Find entry in files
    i = 0;
    while (i < FILES_ENTRY_COUNT && !is_found_parent) {
        clear(filename_buffer, 16);
        strcpybounded(filename_buffer, dirtable+FILES_ENTRY_SIZE*i+PATHNAME_BYTE_OFFSET, 14);
        if (dirtable[i*FILES_ENTRY_SIZE + PARENT_BYTE_OFFSET] == source_dir_idx &&
            !strcmp(source_directory_name, filename_buffer)) {
            is_found_parent = true;
            target_entry_byte = dirtable[i*FILES_ENTRY_SIZE+ENTRY_BYTE_OFFSET];
            target_parent_byte = dirtable[i*FILES_ENTRY_SIZE+PARENT_BYTE_OFFSET];
            link_status = dirtable[i*FILES_ENTRY_SIZE+LINK_BYTE_OFFSET];
            memcpy(filename_buffer, dirtable+i*FILES_ENTRY_SIZE+PATHNAME_BYTE_OFFSET, 14);
            original_softlink_index = i;
        }
        i++;
    }

    if (link_status == SOFTLINK_ENTRY || link_status == HARDLINK_ENTRY) {
        // Will copy softlink
        getDirectoryTable(dirtable);

        // Find empty entry
        i = 0;
        while (i < FILES_ENTRY_COUNT && !empty_entry_found) {
            if (dirtable[i*FILES_ENTRY_SIZE+ENTRY_BYTE_OFFSET] == EMPTY_FILES_ENTRY) {
                empty_entry_found = true;
                dirtable[i*FILES_ENTRY_SIZE+PARENT_BYTE_OFFSET] = target_parent_byte;
                dirtable[i*FILES_ENTRY_SIZE+ENTRY_BYTE_OFFSET] = target_entry_byte;
                clear(filename_buffer, 16);
                strcpybounded(filename_buffer, copied_directory_name, 14);
                memcpy(dirtable+i*FILES_ENTRY_SIZE+PATHNAME_BYTE_OFFSET, filename_buffer, 14);
                dirtable[i*FILES_ENTRY_SIZE+LINK_BYTE_OFFSET] = link_status;
            }
            i++;
        }

        directSectorWrite(dirtable, FILES_SECTOR);
        directSectorWrite(dirtable+SECTOR_SIZE, FILES_SECTOR+1);
        *returncode_cpy = -1;
        *returncode_src = 0;
    }
    else {
        if (target_entry_byte != EMPTY_FILES_ENTRY) {
            read(file_read, source_directory_name, returncode_src, source_dir_idx);
            if (file_read[0] != NULL) {
                print("cp: ", BIOS_WHITE);
                print(source_directory_name, BIOS_LIGHT_GREEN);
                print(" copied\n", BIOS_WHITE);
                write(file_read, copied_directory_name, returncode_cpy, copied_dir_idx);
                *returncode_cpy = -1;
                *returncode_src = 0;
            }
            else {
                print("cp: internal error: ", BIOS_WHITE);
                print(source_directory_name, BIOS_WHITE);
                print(" is a folder\n", BIOS_WHITE);
            }
        }
        else {
            print("cp: ", BIOS_WHITE);
            print(source_directory_name, BIOS_WHITE);
            print(" softlink broken\n", BIOS_WHITE);
        }
    }

}

void cp(char *file_read, char *dirtable, char current_dir_index, char flags, char *target, char *linkname) {
    // Technically just "copy" of previous implementation of ln
    // char as string / char
    char filename_buffer[16];
    char source_directory_name[16];
    char copied_directory_name[16];
    char evaluator_buffer[16];

    // char as 1 byte integer
    char target_entry_byte = 0;
    char target_parent_byte = 0;
    char link_status = -1;
    int stack_source[256]; // <3 stack, but not bcc :(
    int stack_copied[256]; // actually pseudo-stack but whatever
    int copied_parent_dir[256];
    char stack_top_pointer_src;
    char stack_top_pointer_cpy;
    char source_dir_idx;
    char copied_dir_idx;
    char current_recursion_parent_index_src;
    char current_recursion_parent_index_cpy;
    int top_cpy;
    int returncode_src = 0;
    int returncode_cpy = 0;
    bool is_write_success = false, valid_filename = true;
    bool f_target_found = false, empty_entry_found = false;
    bool recursion_loop_pass_success = false;
    bool is_found_parent = false;
    bool is_first_rec = true;
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

        read(file_read, source_directory_name, &returncode_src, current_dir_index);
        if (file_read[0] == FOLDER && flags == 1) {
            clear(evaluator_buffer, 16);
            strcpy(evaluator_buffer, source_directory_name);
            strapp(evaluator_buffer, "/");
            source_dir_idx = directoryEvaluator(dirtable, evaluator_buffer, &returncode_src, current_dir_index);
        }
        else
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

        read(file_read, copied_directory_name, &returncode_cpy, current_dir_index);
        if (file_read[0] == FOLDER && flags == 1) {
            clear(evaluator_buffer, 16);
            strcpy(evaluator_buffer, copied_directory_name);
            strapp(evaluator_buffer, "/");
            copied_dir_idx = directoryEvaluator(dirtable, evaluator_buffer, &returncode_cpy, current_dir_index);
        }
        else
            copied_dir_idx = current_dir_index;

        returncode_cpy = 0;
    }


    // Copying file if path evaluation success
    if (returncode_src == -1) {
        print("cp: ", BIOS_GRAY);
        print(target, BIOS_GRAY);
        print(": target not found\n", BIOS_GRAY);
    }
    else if (returncode_cpy == -1) {
        print("cp: ", BIOS_GRAY);
        print(copied_directory_name, BIOS_GRAY);
        print(": path not found\n", BIOS_GRAY);
    }
    else {
        clear(file_read, FILE_SIZE_MAXIMUM);
        read(file_read, copied_directory_name, &returncode_cpy, copied_dir_idx);
        clear(file_read, FILE_SIZE_MAXIMUM);
        read(file_read, source_directory_name, &returncode_src, source_dir_idx);
        if ((returncode_src == 0 && returncode_cpy == -1) || flags == 1) {
            if (flags == 0) {
                // Simple copy, refuse folder
                if (file_read[0] != NULL) {
                    file_copy(file_read, source_directory_name, &returncode_src, source_dir_idx,
                        copied_directory_name, &returncode_cpy, copied_dir_idx, dirtable);
                    returncode_cpy = -1;
                    returncode_src = 0;
                }
                else {
                    print("cp: error: ", BIOS_WHITE);
                    print(target, BIOS_WHITE);
                    print(" is a folder\n", BIOS_WHITE);
                    returncode_src = -1;
                }
            }
            else {
                // Stolen from rm
                // Will deleting file normally on -r
                if (file_read[0] != NULL) {
                    file_copy(file_read, source_directory_name, &returncode_src, source_dir_idx,
                        copied_directory_name, &returncode_cpy, copied_dir_idx, dirtable);
                }
                else {
                    i = 0;
                    while (i < 256) {
                        copied_parent_dir[i] = 0;
                        stack_source[i] = 0;
                        stack_copied[i] = 0;
                        i++;
                    }
                    stack_top_pointer_src = 0;
                    stack_top_pointer_cpy = 0;
                    stack_source[0] = source_dir_idx;
                    stack_copied[0] = copied_dir_idx;
                    copied_parent_dir[0] = 0;

                    while (!recursion_loop_pass_success) {
                        // FIXME : Extra, Extra, actually cannot copy 2 directory in 1 parent
                        i = 0;
                        recursion_loop_pass_success = true;

                        if (is_first_rec) {
                            // Use original copied directory name on first recursion
                            is_first_rec = false;
                        }
                        else {
                            clear(copied_directory_name, 16);
                            top_cpy = (stack_source[stack_top_pointer_src])*FILES_ENTRY_SIZE;
                            strcpybounded(copied_directory_name, dirtable+top_cpy+PATHNAME_BYTE_OFFSET, 14);
                        }
                        print("cp: ", BIOS_WHITE);
                        print(copied_directory_name, BIOS_LIGHT_BLUE);
                        print(" created\n", BIOS_WHITE);
                        write(FOLDER, copied_directory_name, &returncode_cpy, stack_copied[stack_top_pointer_cpy]);
                        getDirectoryTable(dirtable);
                        current_recursion_parent_index_src = stack_source[stack_top_pointer_src];
                        copied_parent_dir[stack_top_pointer_src] = 0;
                        // Get new file index
                        while (i < FILES_ENTRY_COUNT) {
                            clear(filename_buffer, 16);
                            strcpybounded(filename_buffer, dirtable+i*FILES_ENTRY_SIZE+PATHNAME_BYTE_OFFSET, 14);
                            if (!strcmp(filename_buffer, copied_directory_name) && dirtable[i*FILES_ENTRY_SIZE+PARENT_BYTE_OFFSET] == stack_copied[stack_top_pointer_cpy]) {
                                stack_top_pointer_cpy++;
                                stack_copied[stack_top_pointer_cpy] = i;
                            }
                            i++;
                        }
                        current_recursion_parent_index_cpy = stack_copied[stack_top_pointer_cpy];

                        i = 0;
                        while (i < FILES_ENTRY_COUNT) {
                            if (current_recursion_parent_index_src == dirtable[i*FILES_ENTRY_SIZE+PARENT_BYTE_OFFSET]) {
                                // If still file, pass is still success
                                if (dirtable[i*FILES_ENTRY_SIZE+ENTRY_BYTE_OFFSET] != FOLDER_ENTRY
                                    && dirtable[i*FILES_ENTRY_SIZE+ENTRY_BYTE_OFFSET] != EMPTY_FILES_ENTRY) {
                                    clear(filename_buffer, 16);
                                    strcpybounded(filename_buffer, dirtable+i*FILES_ENTRY_SIZE+PATHNAME_BYTE_OFFSET, 14);
                                    file_copy(file_read, filename_buffer, &returncode_src, current_recursion_parent_index_src,
                                            filename_buffer, &returncode_cpy, current_recursion_parent_index_cpy, dirtable);
                                }
                                // If folder, pass failed
                                else {
                                    stack_top_pointer_src++;
                                    stack_source[stack_top_pointer_src] = (char) i;
                                    copied_parent_dir[stack_top_pointer_src] = current_recursion_parent_index_src;
                                    recursion_loop_pass_success = false;
                                }
                            }
                            i++;
                        }

                        if (recursion_loop_pass_success) {
                            i = stack_top_pointer_src;
                            while (i > 0) {
                                if (copied_parent_dir[i] != 0) {
                                    // Stack unwinding
                                    stack_top_pointer_src = i;
                                    stack_top_pointer_cpy = i;
                                    recursion_loop_pass_success = false;
                                    break;
                                }
                                i--;
                            }
                        }
                    }


                    returncode_cpy = -1;
                    returncode_src = 0;
                    print("cp: recursion done\n", BIOS_WHITE);
                }
            }

        }
        else {
            print("cp: error: ", BIOS_WHITE);
            print(target, BIOS_WHITE);
            print(" not found\n", BIOS_WHITE);
            returncode_src = -1;
        }

        if (returncode_src == 0 && returncode_cpy == -1) {
            print(linkname, BIOS_GRAY);
            print(": copy created\n", BIOS_GRAY);
        }
        else
            print("cp: file writing error\n", BIOS_GRAY);
    }
}
