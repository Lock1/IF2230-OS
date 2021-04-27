#include "kernel-header/config.h"
#include "std-header/std_stringio.h"
#include "shell-header/shell_common.h"
#include "basic-header/std_opr.h"
#include "std-header/boolean.h"

#define CTRL_C_SCANCODE_LOW 0x03
#define CTRL_C_SCANCODE_HIGH 0x2E

void mim_editor(char *target_filename, char current_dir_index, char *dirtable);

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
    if (argc == 2)
        if (!strcmp("--help", arg_vector[0])) {
            print("Utility to create/edit file in mim text editor\n", BIOS_WHITE);
            print("Possible Usage:\n", BIOS_LIGHT_BLUE);
            print("mim [file_name]\n", BIOS_LIGHT_CYAN);
        }
        else {
            mim_editor(arg_vector[0], current_dir_index, directory_table);
        }
    else
        print("Usage : mim <source>\n", BIOS_WHITE);

    setShellCache(shell_cache);
    shellReturn();
}

void print_title(char *title) {
    int saved_r = getKeyboardCursor(true);
    int saved_c = getKeyboardCursor(false);
    setKeyboardCursor(0, 0);
    print(title, BIOS_YELLOW);
    setKeyboardCursor(saved_r, saved_c);
}

void editor(char *file_buffer, char *title) {
    // char as 1 byte integer
    char c, scancode;
    int i = 0, j = 0, max_i = 0, rawKey;
    int savedCursorRow = getKeyboardCursor(true);
    int savedCursorCol = getKeyboardCursor(false);

    clearEntireScreen();
    print_title(title);
    setKeyboardCursor(1, 0);
    showKeyboardCursor();

    print(file_buffer, BIOS_GRAY);
    savedCursorRow = getKeyboardCursor(true);
    savedCursorCol = getKeyboardCursor(false);
    i = strlenbin(file_buffer) - 5;
    max_i = i;
    do {
        if (getKeyboardCursor(true) > 20)
            scrollScreen();
        rawKey = getFullKey();
        c = rawKey & 0xFF;      // AL Value
        scancode = rawKey >> 8; // AH Value
        // WARNING : Prioritizing ASCII before scancode
        if (!(scancode == CTRL_C_SCANCODE_HIGH && c == CTRL_C_SCANCODE_LOW)) {
            switch (c) {
                case CHAR_INPUT_NEWLINE:
                    file_buffer[i] = CHAR_LINEFEED;
                    i++;
                    savedCursorRow = getKeyboardCursor(true);
                    savedCursorCol = getKeyboardCursor(false);
                    setKeyboardCursor(savedCursorRow + 1, 0);
                    break;
                case CHAR_BACKSPACE:
                    // If i is not at starting input pos, decrement
                    if (i == max_i) {
                        if (i > 0)
                            // TODO : Extra, Extra, Extra, Extra, printing on edge screen sucks
                            i--;

                            if (max_i > 0)
                            max_i--;

                            savedCursorRow = getKeyboardCursor(true);
                            savedCursorCol = getKeyboardCursor(false);
                            file_buffer[max_i] = CHAR_SPACE; // For deleting last char
                            file_buffer[max_i+1] = CHAR_NULL;
                            setKeyboardCursor(1, 0);
                            print(file_buffer, BIOS_GRAY);

                            setKeyboardCursor(savedCursorRow, savedCursorCol-1);
                        }
                        break;
                default:
                    // If char (AL) is not ASCII Control codes, check scancode (AH)
                    switch (scancode) {
                        //                     case SCANCODE_TAB:
                        //                         // Deleted auto complete
                        //                         break;
                        //                     case SCANCODE_DOWN_ARROW:
                        //                     case SCANCODE_UP_ARROW:
                        //                         // Deleted auto complete
                        //                         break;
                        case SCANCODE_LEFT_ARROW:
                            if (i > 0)
                                i--;
                            break;
                        case SCANCODE_RIGHT_ARROW:
                            if (i < max_i)
                                i++;
                            break;
                        default:
                            putchar(c);
                            file_buffer[i] = c;
                            if (i == max_i)
                                max_i++;
                            i++;
                    }
                    // savedCursorRow = getKeyboardCursor(true);
                    // savedCursorCol = getKeyboardCursor(false);
                // setKeyboardCursor(savedCursorRow, savedCursorCol + 1);
            }

        }
    } while (!(scancode == CTRL_C_SCANCODE_HIGH && c == CTRL_C_SCANCODE_LOW));

    clearEntireScreen();
    setKeyboardCursor(0, 0);
    print("mim: ", BIOS_WHITE);
    print(title, BIOS_LIGHT_GREEN);
    print(" saved\n", BIOS_WHITE);
}


void mim_editor(char *target_filename, char current_dir_index, char *dirtable) {
    // char as string / char
    char filename_buffer[16];
    char source_directory_name[16];
    char string_word_count[32];
    // char as 1 byte integer
    char file_read[FILE_SIZE_MAXIMUM];
    char target_entry_byte = 0;
    char target_parent_byte = 0;
    char source_dir_idx;
    char link_status = -1;
    int returncode_editor = -1;
    int returncode_src = 0;
    bool is_write_success = false, valid_filename = true;
    bool f_target_found = false, empty_entry_found = false;
    bool is_found_parent = false;
    int i = 0, j = 0;
    int f_entry_idx = 0;
    int f_entry_sector_idx = 0;
    int last_slash_index;
    int word_count;
    int null_count;

    // Relative pathing
    clear(source_directory_name, 16);
    // Split target / source
    last_slash_index = getLastMatchedCharIdx(CHAR_SLASH, target_filename);
    // FIXME : Extra, unsafe getlast
    if (last_slash_index != -1) {
        // Split argument to path and filename
        // Get path
        strcpybounded(source_directory_name, target_filename, last_slash_index);
        source_dir_idx = directoryEvaluator(dirtable, source_directory_name, &returncode_src, current_dir_index);

        // Get filename
        strcpybounded(source_directory_name, target_filename+last_slash_index+1, ARG_LENGTH-last_slash_index-1);
    }
    else {
        // Cut slash
        strcpybounded(source_directory_name, target_filename, ARG_LENGTH);

        source_dir_idx = current_dir_index;
        returncode_src = 0;
    }

    // Copying strings if path evaluation success
    if (returncode_src == -1) {
        print("mim: ", BIOS_GRAY);
        print(target_filename, BIOS_GRAY);
        print(": target path not found\n", BIOS_GRAY);
    }
    else {
        clear(file_read, FILE_SIZE_MAXIMUM);
        read(file_read, source_directory_name, &returncode_src, source_dir_idx);
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
                target_entry_byte = dirtable[target_entry_byte*FILES_ENTRY_SIZE + ENTRY_BYTE_OFFSET];
                clear(file_read, FILE_SIZE_MAXIMUM);
                read(file_read, filename_buffer, &returncode_src, target_parent_byte);
                // Needed to compare due _mash_cache always filling empty space
                // Implying softlink to _mash_cache is not available
                if (!strcmp(filename_buffer, "_mash_cache"))
                    target_entry_byte = EMPTY_FILES_ENTRY;
            }

            if (returncode_src == 0 && target_entry_byte != EMPTY_FILES_ENTRY && target_entry_byte != FOLDER_ENTRY) {
                // Editing existing file
                editor(file_read, source_directory_name);
                remove(source_directory_name, &returncode_editor, source_dir_idx);
                write(file_read, source_directory_name, &returncode_editor, source_dir_idx);
            }
            else {
                print("mim: ", BIOS_WHITE);
                print(source_directory_name, BIOS_WHITE);
                print(" softlink broken\n", BIOS_WHITE);
            }

        }
        else {
            // Creating new
            clear(file_read, FILE_SIZE_MAXIMUM);
            editor(file_read, source_directory_name);
            write(file_read, source_directory_name, &returncode_editor, source_dir_idx);
        }


    }
}
