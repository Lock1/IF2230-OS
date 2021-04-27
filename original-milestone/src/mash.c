#include "std-header/std_fileio.h"
#include "std-header/std_stringio.h"
#include "basic-header/std_opr.h"
#include "kernel-header/config.h"
#include "std-header/boolean.h"
#include "shell-header/shell_common.h"

void shell();

// Shell convention
// cache 0-0xF reserved for argv passing and shell state
// cache[0xF] used as current working directory
// cache 1-0xF will filled with evaluated argv
// cache 0x10-0x4F, 0x50-0x8F, 0x90-0xCF, 0xD0-0x10F, 0x110-0x14F
//      used as shell history
// rest of cache used as argv

// BIOS_LIGHT_BLUE indicate folder
// BIOS_LIGHT_GREEN indicate file
// BIOS_LIGHT_CYAN indicate softlink
// BIOS_LIGHT_RED indicate hardlink

// CTRL + F / find Activate autocompletion for adding directory
//      autocomplete for arbitrary commands

int main() {
    char cache_buffer[SECTOR_SIZE];
    int returncode;
    // Cache reading
    clear(cache_buffer, SECTOR_SIZE);
    getShellCache(cache_buffer);
    showKeyboardCursor();

    // Empty cache case
    if (!forcestrcmp(EMPTY_CACHE, cache_buffer)) {
        clear(cache_buffer, SECTOR_SIZE);
        cache_buffer[CURRENT_DIR_CACHE_OFFSET] = ROOT_PARENT_FOLDER;
        cache_buffer[CACHE_SIGNATURE_OFFSET] = CACHE_SIGNATURE;
    }

    setShellCache(cache_buffer);
    shell(cache_buffer);
    while (1);
}

void directoryStringBuilder(char *string, char *dirtable, char current_dir) {
    // Use as string / char
    char filename_buffer[16];
    // Use as 1 bytes integer
    char current_parent = 0, parent[FILES_ENTRY_COUNT];
    // parent will contain indices in reversed order
    int i = 0, parent_length = 0;
    if (current_dir == ROOT_PARENT_FOLDER)
        string[0] = '/';
    else {
        clear(parent, FILES_ENTRY_COUNT);
        // Traversing folder until reaching root
        current_parent = dirtable[current_dir*FILES_ENTRY_SIZE+PARENT_BYTE_OFFSET];
        while (current_parent != ROOT_PARENT_FOLDER) {
            parent[parent_length] = current_parent;
            parent_length++;
            current_parent = dirtable[current_parent*FILES_ENTRY_SIZE+PARENT_BYTE_OFFSET];
        }

        // Adding lower parent
        i = parent_length - 1;
        while (i >= 0) {
            strapp(string, "/");
            clear(filename_buffer, 16);
            strcpybounded(filename_buffer, dirtable+(parent[i]*FILES_ENTRY_SIZE)+PATHNAME_BYTE_OFFSET, 14);
            strapp(string, filename_buffer);

            i--;
        }

        // Adding topmost parent
        strapp(string, "/");
        clear(filename_buffer, 16);
        strcpybounded(filename_buffer, dirtable+current_dir*FILES_ENTRY_SIZE+PATHNAME_BYTE_OFFSET, 14);
        strapp(string, filename_buffer);
    }
}

void shellInput(char *commands_history, char *dirtable, char current_dir) {
    // char as string
    char string[BUFFER_SIZE];
    char move_string_buffer_1[BUFFER_SIZE];
    char move_string_buffer_2[BUFFER_SIZE];
    char arg_vector[ARGC_MAX][ARG_LENGTH];
    char temp_eval[ARGC_MAX*ARG_LENGTH];
    char to_be_completed[ARGC_MAX*ARG_LENGTH];
    char filename_buffer[16];
    // char as 1 byte integer
    char c, scancode;
    int i = 0, j = 0, max_i = 0, rawKey, dbg = 0;
    int split_i, split_j, split_k;
    int selected_his_idx = 0, current_eval_idx = 0;
    int savedCursorRow = getKeyboardCursor(1);
    int savedCursorCol = getKeyboardCursor(0);
    bool is_modified = false, is_autocomplete_available = false;
    bool is_between_quote_mark = false, autocomplete_found = false;
    int argc = 0, returncode = 0, matched_idx = 0;
    int target_copy_arg_idx;
    showKeyboardCursor();

    // Move history up
    strcpybounded(move_string_buffer_1, commands_history, BUFFER_SIZE - 1);
    while (i < MAX_HISTORY - 1) {
        strcpybounded(move_string_buffer_2, commands_history+BUFFER_SIZE*(i+1), BUFFER_SIZE - 1);
        strcpybounded(commands_history+BUFFER_SIZE*(i+1), move_string_buffer_1, BUFFER_SIZE - 1);
        strcpybounded(move_string_buffer_1, move_string_buffer_2, BUFFER_SIZE - 1);
        i++;
    }
    clear(commands_history, BUFFER_SIZE); // Delete first entry

    i = 0;
    selected_his_idx = 0;
    do {
        rawKey = getFullKey();
        c = rawKey & 0xFF;      // AL Value
        scancode = rawKey >> 8; // AH Value
        // WARNING : Prioritizing ASCII before scancode
        switch (c) {
            case CHAR_INPUT_NEWLINE:
                break;
            case CHAR_BACKSPACE:
                is_modified = true;
                // If i is not at starting input pos, decrement
                if (i > 0)
                    i--;

                // Shift copy from deleted index
                j = i;
                while (j < max_i) {
                    string[j] = string[j + 1];
                    j++;
                }

                // If buffer not empty, decrement size by 1
                if (max_i > 0)
                    max_i--;

                string[max_i] = CHAR_SPACE; // For deleting last char
                string[max_i+1] = CHAR_NULL;
                setKeyboardCursor(savedCursorRow, savedCursorCol);
                print(string, BIOS_GRAY);

                setKeyboardCursor(savedCursorRow, savedCursorCol + i);
                break;
            default:
                // If char (AL) is not ASCII Control codes, check scancode (AH)
                switch (scancode) {
                    case SCANCODE_TAB:
                        // Note : Currently autocomplete not available for inside quote
                        // Part 1: command identification
                        // TODO : Extra, need strsplit so badly :(
                        // TODO : Extra, Extra, sometimes print failed (?)
                        // Arguments splitting
                        // Temporary cut string
                        string[max_i] = CHAR_NULL;
                        argc = 0;
                        clear(arg_vector, ARGC_MAX*ARG_LENGTH);
                        is_between_quote_mark = false;
                        split_i = 0;
                        split_j = 0;
                        split_k = 0;

                        while (string[split_i] != CHAR_NULL) {
                            // TODO : Extra, Extra, mixing double and single quote
                            // If found space in commands and not within double quote mark, make new
                            if (string[split_i] == CHAR_SPACE && split_j < ARGC_MAX && !is_between_quote_mark) {
                                split_k = 0;
                                split_j++;
                            }
                            else if (string[split_i] == CHAR_DOUBLE_QUOTE) {
                                // Toggling is_between_quote_mark
                                is_between_quote_mark = !is_between_quote_mark;
                            }
                            else {
                                // Only copy if char is not double quote
                                // and space outside double quote
                                arg_vector[split_j][split_k] = string[split_i];
                                split_k++;
                            }

                            split_i++;
                        }
                        argc = split_j + 1; // Due split_j is between counting space between 2 args

                        is_autocomplete_available = false;
                        // ------ Activate autocompletion feature here ------
                        if (argc == 4 &&
                            (  isLastSubstring(arg_vector[0], "cp") || isLastSubstring(arg_vector[0], "mv")
                            || isLastSubstring(arg_vector[0], "ln"))
                            ) {
                            // Fourth arg autocomplete
                            target_copy_arg_idx = 3;
                            is_autocomplete_available = true;
                        }
                        else if (argc == 3 &&
                            (  isLastSubstring(arg_vector[0], "cp") || isLastSubstring(arg_vector[0], "mv")
                            || isLastSubstring(arg_vector[0], "ln") || isLastSubstring(arg_vector[0], "rm"))
                            ) {
                            // Third arg autocomplete
                            target_copy_arg_idx = 2;
                            is_autocomplete_available = true;
                        }
                        else if (isLastSubstring(arg_vector[0], "ls") || isLastSubstring(arg_vector[0], "cat")
                            || isLastSubstring(arg_vector[0], "cd") || isLastSubstring(arg_vector[0], "cp")
                            || isLastSubstring(arg_vector[0], "mv") || isLastSubstring(arg_vector[0], "ln")
                            || isLastSubstring(arg_vector[0], "rm") || isLastSubstring(arg_vector[0], "file")
                            || isLastSubstring(arg_vector[0], "wc")) {
                            // Second arg autocomplete
                            target_copy_arg_idx = 1;
                            is_autocomplete_available = true;
                        }
                        else if (!forcestrcmp("./", arg_vector[0]) && argc == 1) {
                            // First arg autocomplete
                            target_copy_arg_idx = 0;
                            is_autocomplete_available = true;
                        }

                        if (!is_between_quote_mark && is_autocomplete_available) {
                            // Part 2: current index evaluation
                            current_eval_idx = current_dir;
                            matched_idx = getLastMatchedCharIdx(CHAR_SLASH, arg_vector[target_copy_arg_idx]);
                            // If argv[1] is only single name, use original dir
                            if (matched_idx != -1) {
                                clear(temp_eval,ARGC_MAX*ARG_LENGTH);
                                strcpybounded(temp_eval, arg_vector[target_copy_arg_idx], matched_idx);
                                current_eval_idx = directoryEvaluator(dirtable, temp_eval, &returncode, current_dir);
                            }


                            // Part 3: command autocompletion
                            // "To be completed" command (ex. cat mnt/abc/pqr -> pqr)
                            clear(to_be_completed, ARGC_MAX*ARG_LENGTH);
                            strcpy(to_be_completed, arg_vector[target_copy_arg_idx]+matched_idx+1);
                            // Searching from directory table
                            autocomplete_found = false;
                            split_i = 0;
                            while (split_i < FILES_ENTRY_COUNT && !autocomplete_found) {
                                clear(filename_buffer, 16);
                                strcpybounded(filename_buffer, dirtable+FILES_ENTRY_SIZE*split_i+PATHNAME_BYTE_OFFSET, 14);
                                if (current_eval_idx == dirtable[FILES_ENTRY_SIZE*split_i+PARENT_BYTE_OFFSET]) {
                                    // Partial string comparation
                                    split_j = 0;
                                    autocomplete_found = true;
                                    // Set autocomplete_found as found, if string comparation below failed
                                    //       cancel searching status to not found
                                    while (to_be_completed[split_j] != CHAR_NULL && autocomplete_found) {
                                        if (to_be_completed[split_j] != filename_buffer[split_j])
                                            autocomplete_found = false;
                                        split_j++;
                                    }
                                }
                                split_i++;
                            }

                            if (autocomplete_found) {
                                if (matched_idx != -1) {
                                    // If using relative pathing, then find last slash location and insert completion
                                    strcpy(string+getLastMatchedCharIdx(CHAR_SLASH, string)+1, filename_buffer);
                                }
                                else {
                                    // Autocomplete for n-arg
                                    // If not using relative pathing, use space as insertion location
                                    strcpy(string+getLastMatchedCharIdx(CHAR_SPACE, string)+1, filename_buffer);
                                }
                            }

                            // Autocomplete printing
                            setKeyboardCursor(savedCursorRow, savedCursorCol);
                            print("                                                                ", BIOS_GRAY);
                            setKeyboardCursor(savedCursorRow, savedCursorCol);

                            // Change proper i and max_i values
                            i = strlen(string);
                            max_i = i;

                            print(string, BIOS_GRAY);
                        }
                        break;
                    case SCANCODE_DOWN_ARROW:
                    case SCANCODE_UP_ARROW:
                        if (scancode == SCANCODE_DOWN_ARROW && selected_his_idx > 0)
                            selected_his_idx--;
                        else if (scancode == SCANCODE_UP_ARROW && selected_his_idx < MAX_HISTORY - 1)
                            selected_his_idx++;

                        setKeyboardCursor(savedCursorRow, savedCursorCol);
                        print("                                                                ", BIOS_GRAY);
                        setKeyboardCursor(savedCursorRow, savedCursorCol);
                        // Move current buffer to history first location, only if string is modified
                        if (is_modified) {
                            string[max_i] = CHAR_NULL;
                            strcpybounded(commands_history, string, BUFFER_SIZE - 1);
                        }

                        // Load command from history
                        strcpybounded(string, commands_history+(selected_his_idx*BUFFER_SIZE), BUFFER_SIZE - 1);
                        // Change proper i and max_i values
                        i = strlen(string);
                        max_i = i;

                        print(commands_history+(selected_his_idx*BUFFER_SIZE), BIOS_GRAY);
                        is_modified = false;
                        break;
                    case SCANCODE_LEFT_ARROW:
                        if (i > 0)
                            i--;
                        break;
                    case SCANCODE_RIGHT_ARROW:
                        if (i < max_i)
                            i++;
                        break;
                    default:
                        is_modified = true;
                        putchar(c);
                        string[i] = c;
                        if (i == max_i)
                            max_i++;
                        i++;
                }
                setKeyboardCursor(savedCursorRow, savedCursorCol + i);
        }
    } while (c != CHAR_INPUT_NEWLINE);
    string[max_i] = CHAR_NULL; // Terminating string
    hideKeyboardCursor();
    setKeyboardCursor(savedCursorRow + 1, 0); // TODO : Extra, for multi-line input, maybe can be adjusted

    strcpybounded(commands_history, string, BUFFER_SIZE - 1);
}

void shell(char *cache) {
    // char as string / char
    char filename_buffer[16];
    char source_directory_name[16];
    char commands_history[MAX_HISTORY][BUFFER_SIZE]; // "FIFO" data type for commands
    char directory_string[BUFFER_SIZE];
    char arg_vector[ARGC_MAX][ARG_LENGTH];
    char directory_table[2][SECTOR_SIZE];
    char arg_execute[ARG_LENGTH];
    // char as 1 byte integer
    char target_entry_byte = 0;
    char target_parent_byte = 0;
    char source_dir_idx;
    char link_status = -1;
    char io_buffer[SECTOR_SIZE];
    char temp_file[FILE_SIZE_MAXIMUM];
    char current_dir_index = cache[CURRENT_DIR_CACHE_OFFSET];
    char is_between_quote_mark = false;
    bool is_found_parent = false;
    int returncode_src;
    int temp, returncode;
    char evaluated_dir_idx = current_dir_index;
    int last_execute_slash_index = 0;
    int i = 0, j = 0, k = 0, argc = 0;

    getDirectoryTable(directory_table);

    clear(commands_history, BUFFER_SIZE*MAX_HISTORY);
    memcpy(commands_history, cache+HISTORY_CACHE_OFFSET, BUFFER_SIZE*MAX_HISTORY);

    while (true) {
        clear(arg_vector, ARGC_MAX*ARG_LENGTH);
        clear(directory_string, BUFFER_SIZE);
        print("mangga", BIOS_GREEN);
        print(":", BIOS_GRAY);
        directoryStringBuilder(directory_string, directory_table, current_dir_index);
        print(directory_string, BIOS_LIGHT_BLUE);
        print("$ ", BIOS_GRAY);
        shellInput(commands_history, directory_table, current_dir_index);
        memcpy(cache+HISTORY_CACHE_OFFSET, commands_history, BUFFER_SIZE*MAX_HISTORY);

        // Scroll up if cursor at lower screen
        while (getKeyboardCursor(1) > 20) {
            scrollScreen();
            setKeyboardCursor(getKeyboardCursor(1)-1, 0);
            showKeyboardCursor();
        }

        // Arguments splitting
        i = 0;
        j = 0;
        k = 0;
        while (commands_history[0][i] != CHAR_NULL) {
            // TODO : Extra, Extra, mixing double and single quote
            // If found space in commands and not within double quote mark, make new
            if (commands_history[0][i] == CHAR_SPACE && j < ARGC_MAX && !is_between_quote_mark) {
                k = 0;
                j++;
            }
            else if (commands_history[0][i] == CHAR_DOUBLE_QUOTE) {
                // Toggling is_between_quote_mark
                is_between_quote_mark = !is_between_quote_mark;
            }
            else {
                // Only copy if char is not double quote
                // and space outside double quote
                arg_vector[j][k] = commands_history[0][i];
                k++;
            }

            i++;
        }
        argc = j + 1; // Due j is between counting space between 2 args

        clear(cache+ARGV_OFFSET, ARG_LENGTH);
        memcpy(cache+ARGV_OFFSET, arg_vector[1], ARG_LENGTH);
        memcpy(cache+ARGV_2_OFFSET, arg_vector[2], ARG_LENGTH);
        memcpy(cache+ARGV_3_OFFSET, arg_vector[3], ARG_LENGTH);
        cache[ARGC_OFFSET] = argc;
        setShellCache(cache);
        clear(temp_file, FILE_SIZE_MAXIMUM);
        // Command evaluation, TODO : Move to program itself
        if (!forcestrcmp("./", arg_vector[0])) {
            // Do relative pathing if more than 1
            last_execute_slash_index = getLastMatchedCharIdx(CHAR_SLASH, arg_vector[0]);
            // FIXME : Extra, unsafe getlast
            clear(arg_execute, ARG_LENGTH);
            if (last_execute_slash_index != 1) {
                // Split argument to path and filename
                // Get path
                strcpybounded(arg_execute, arg_vector[0], last_execute_slash_index);
                evaluated_dir_idx = directoryEvaluator(directory_table, arg_execute, &returncode, current_dir_index);

                // Get filename
                strcpybounded(arg_execute, arg_vector[0]+last_execute_slash_index+1, ARG_LENGTH-last_execute_slash_index-1);
            }
            else {
                // Cut slash
                strcpybounded(arg_execute, arg_vector[0]+2, ARG_LENGTH-2);

                evaluated_dir_idx = current_dir_index;
                returncode = 0;
            }

            // Link check
            // Find entry in files
            i = 0;
            is_found_parent = false;
            while (i < FILES_ENTRY_COUNT && !is_found_parent) {
                clear(filename_buffer, 16);
                strcpybounded(filename_buffer, directory_table[0]+FILES_ENTRY_SIZE*i+PATHNAME_BYTE_OFFSET, 14);
                if (directory_table[0][i*FILES_ENTRY_SIZE+PARENT_BYTE_OFFSET] == evaluated_dir_idx &&
                    !forcestrcmp(filename_buffer, arg_execute)) {
                    is_found_parent = true;
                    target_entry_byte = directory_table[0][i*FILES_ENTRY_SIZE+ENTRY_BYTE_OFFSET];
                    link_status = directory_table[0][i*FILES_ENTRY_SIZE+LINK_BYTE_OFFSET];

                }
                i++;
            }

            if (link_status == SOFTLINK_ENTRY) {
                // TODO : Extra, currently only single softlink depth supported
                clear(filename_buffer, 16);
                strcpybounded(filename_buffer, directory_table[0]+target_entry_byte*FILES_ENTRY_SIZE+PATHNAME_BYTE_OFFSET, 14);
                strcpybounded(arg_execute, filename_buffer, 14);
                target_parent_byte = directory_table[0][target_entry_byte*FILES_ENTRY_SIZE + PARENT_BYTE_OFFSET];
                evaluated_dir_idx = target_parent_byte;
                target_entry_byte = directory_table[0][target_entry_byte*FILES_ENTRY_SIZE + ENTRY_BYTE_OFFSET];
                clear(temp_file, FILE_SIZE_MAXIMUM);
                read(temp_file, filename_buffer, &returncode_src, target_parent_byte);
                // Needed to compare due _mash_cache always filling empty space
                // Implying softlink to _mash_cache is not available
                print("mash: executing softlink to ", BIOS_WHITE);
                print(arg_execute, BIOS_LIGHT_CYAN);
                print("\n", BIOS_WHITE);

                if (!strcmp(filename_buffer, "_mash_cache"))
                    target_entry_byte = EMPTY_FILES_ENTRY;
            }


            // Preventing to loading empty arg_execute
            if (target_entry_byte != EMPTY_FILES_ENTRY) {
                // Empty arg_execute will cause load kernel / restarting OS
                read(temp_file, arg_execute, &returncode, evaluated_dir_idx);
                if (!strcmp("./", arg_vector[0]))
                    print("unknown command", BIOS_LIGHT_RED);
                else if (returncode == 0 && isBinaryFileMagicNumber(temp_file))
                    exec(arg_execute, 0x3000, evaluated_dir_idx);
                // If executed, this code wont run
                print(arg_execute, BIOS_WHITE);
                print(": binary signature not found\n", BIOS_WHITE);
            }
            else {
                print("mash: ", BIOS_WHITE);
                print(arg_execute, BIOS_WHITE);
                print(" softlink broken", BIOS_WHITE);
            }
        }
        else if (!strcmp("echo", arg_vector[0])) {
            // Because shell structure is simple, just handle echo here
            if (argc <= 2)
                print(arg_vector[1], BIOS_WHITE);
            else if (!strcmp(">", arg_vector[2])) { // Sad redirection
                clear(io_buffer, SECTOR_SIZE);
                strcpybounded(io_buffer, arg_vector[1], SECTOR_SIZE);
                write(io_buffer, arg_vector[3], &returncode, current_dir_index);
                if (returncode == -1) {
                    print("echo: ", BIOS_WHITE);
                    print(arg_vector[3], BIOS_WHITE);
                    print(" exist ", BIOS_WHITE);
                }
                else {
                    // If success writing to file, load new updated dirtable
                    getDirectoryTable(directory_table);
                }
            }
        }
        else if (!strcmp("clear", arg_vector[0])) {
            clearEntireScreen();
            setKeyboardCursor(0, 0);
        }
        else if (!strcmp("", arg_vector[0])) {
            // WARNING : Multiple space in single block will count as multiple argument due to argsplit above
            // FIXME : Extra, ^ fix this argsplitter
            // Empty string -> doing nothing
        }
        else {
            // WARNING : Softlink in bin/ will executing arbitrary pointed sectors as program
            //           / Shell will ignore softlink flag and execute S byte as sectors entry index
            read(temp_file, arg_vector[0], &returncode, BIN_PARENT_FOLDER);
            if (returncode == 0 && isBinaryFileMagicNumber(temp_file))
                exec(arg_vector[0], 0x3000, BIN_PARENT_FOLDER);
            // If executed, this code wont run
            print(arg_vector[0], BIOS_WHITE);
            print(": command not found\n", BIOS_WHITE);
        }
    }

}
