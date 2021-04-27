// 13519214 - Shell
// TODO : Extra, write to special sector dedicated for history
// TODO : Extra, Extra, special sector for configuration
// TODO : Extra, Extra, Extra, use struct if bcc support struct keyword
// TODO : Extra, Extra, Extra, redirection
// Note : Need interrupt() if linked without kernel, check other/interrupt.asm

#include "kernel-header/config.h" // Only for BIOS Color
#include "std-header/boolean.h"
#include "std-header/std.h"

#define ARG_LENGTH 32
#define ARGC_MAX 8
#define BUFFER_SIZE 256
#define MAX_HISTORY 5

// TODO : Extra, Extra, actually splitting to separate "app"
void getDirectoryTable(char *buffer);
// WARNING : No bound checking
// Get all directory table, put in buffer
void shell();


// int main() {
//     shell();
//     return 0;
// }



void getDirectoryTable(char *buffer) {
    // WARNING : Naive implementation
    interrupt(0x21, 0x0002, buffer, FILES_SECTOR, 0);
    interrupt(0x21, 0x0002, buffer + SECTOR_SIZE, FILES_SECTOR + 1, 0);
}

char directoryEvaluator(char *dirtable, char *dirstr, int *returncode, char current_dir) {
    char evaluated_dir = current_dir;
    char parent_byte_buffer = -1;
    char directory_name[ARGC_MAX][ARG_LENGTH];
    char filename_buffer[16];
    int i, j, k, dirnamecount;
    bool is_valid_args = true, is_folder_found = false;
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
        if (!strcmp(directory_name[i], "."))
            evaluated_dir = current_dir;
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
                if (!strcmp(directory_name[i], filename_buffer) && parent_byte_buffer == evaluated_dir) {
                    is_folder_found = true;
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
    else
        *returncode = 0;

    return evaluated_dir;
}

void fillBuffer(char *buffer, int count, char filler) {
    int i = 0;
    while (i < count) {
        buffer[i] = filler;
        i++;
    }
}

void directoryStringBuilder(char *string, char *dirtable, char current_dir) {
    // Use as string / char
    char filename_buffer[16];
    // Use as 1 bytes integer
    char current_parent = 0, parent[FILES_ENTRY_COUNT];
    // parent will contain indexes in reversed order
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
                        if (!strcmp("ls",arg_vector[0]) || !strcmp("cat",arg_vector[0]) || !strcmp("cd",arg_vector[0]))
                            is_autocomplete_available = true;

                        if (!is_between_quote_mark && is_autocomplete_available) {
                            // Part 2: current index evaluation

                            current_eval_idx = current_dir;
                            matched_idx = getLastMatchedCharIdx(CHAR_SLASH, arg_vector[1]);
                            // If argv[1] is only single name, use original dir
                            if (matched_idx != -1) {
                                clear(temp_eval,ARGC_MAX*ARG_LENGTH);
                                strcpybounded(temp_eval, arg_vector[1], returncode);
                                current_eval_idx = directoryEvaluator(dirtable, temp_eval, &returncode, current_dir);
                            }

                            // Part 3: command autocompletion
                            // "To be completed" command (ex. cat mnt/abc/pqr -> pqr)
                            clear(to_be_completed, ARGC_MAX*ARG_LENGTH);
                            strcpy(to_be_completed, arg_vector[1]+matched_idx+1);
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
                                    // If not using relative pathing, use space as insertion location
                                    strcpy(string+getFirstMatchedCharIdx(CHAR_SPACE, string)+1, filename_buffer);
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


// TODO : Extra, Split file
void ls(char *dirtable, char target_dir) {
    int i = 0;
    // char as string / char
    char filename_buffer[16];
    // Use char as 1 byte integer
    char parent_byte_entry, entry_byte_entry;
    while (i < FILES_ENTRY_COUNT) {
        parent_byte_entry = dirtable[FILES_ENTRY_SIZE*i+PARENT_BYTE_OFFSET];
        entry_byte_entry = dirtable[FILES_ENTRY_SIZE*i+ENTRY_BYTE_OFFSET];
        if (parent_byte_entry == target_dir && entry_byte_entry != EMPTY_FILES_ENTRY) {
            clear(filename_buffer, 16);
            strcpybounded(filename_buffer, dirtable+FILES_ENTRY_SIZE*i+PATHNAME_BYTE_OFFSET, 14);
            if (isCharInString(CHAR_SPACE, filename_buffer)) {
                print("\"");
                print(filename_buffer, BIOS_LIGHT_GREEN);
                print("\"");
            }
            else
                print(filename_buffer, BIOS_LIGHT_GREEN);
            print(" ");
        }
        i++;
    }
    print("\n");
}

char cd(char *dirtable, char *dirstr, char current_dir) {
    int returncode = 0;
    char new_dir_idx = directoryEvaluator(dirtable, dirstr, &returncode, current_dir);
    // If success return new dir index
    if (returncode == 0) {
        if (dirtable[new_dir_idx*FILES_ENTRY_SIZE+ENTRY_BYTE_OFFSET] == FOLDER_ENTRY)
            return new_dir_idx;
        else {
            // Else, entry is not folder
            print("cd: target type is a file\n", BIOS_WHITE);
            return current_dir;
        }
    }
    else {
        // Else, return original dir
        print("cd: path not found\n", BIOS_WHITE);
        return current_dir;
    }
}

void cat(char *dirtable, char *filename, char target_dir) {
    char file_read[FILE_SIZE_MAXIMUM];
    char directory_name[ARGC_MAX][ARG_LENGTH];
    char evaluator_temp[ARGC_MAX*ARG_LENGTH];
    char eval_dir = target_dir;
    int returncode = -1, eval_returncode = -1;
    int dirnamecount;
    int i, j, k;
    char dbg[128];

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

    if (dirnamecount > 1)
        eval_dir = directoryEvaluator(dirtable, evaluator_temp, &eval_returncode, target_dir);

    clear(file_read, FILE_SIZE_MAXIMUM);
    // Take last argv, use it as filename
    read(file_read, directory_name[dirnamecount-1], &returncode, eval_dir);
    if (returncode == -1) {
        print("cat: ", BIOS_GRAY);
        print(directory_name[dirnamecount-1], BIOS_GRAY);
        print(": file not found", BIOS_GRAY);
    }
    else {
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
    print("\n", BIOS_GRAY);
}

void ln(char *dirtable, char target_dir, char flags, char *target, char *linkname) {
    // char as string / char
    char filename_buffer[16];
    // char as 1 byte integer
    char file_read[FILE_SIZE_MAXIMUM];
    char target_entry_byte = 0;
    int returncode = 0;
    int target_sector = div(target_dir, FILES_ENTRY_COUNT/FILES_SECTOR_SIZE);
    int entry_idx_offset_in_sector = mod(target_dir, FILES_ENTRY_COUNT/FILES_SECTOR_SIZE);
    bool is_write_success = false, valid_filename = true;
    bool f_target_found = false, empty_entry_found = false;
    int i = 0, j = 0;
    int f_entry_idx = 0;
    int f_entry_sector_idx = 0;

    // For simplicity, only 2 flags either hard / soft link available
    // For more fancy version, use bitmasking
    clear(file_read, FILE_SIZE_MAXIMUM);
    read(file_read, target, &returncode, target_dir);
    if (returncode == -1) {
        print("ln: ", BIOS_GRAY);
        print(target, BIOS_GRAY);
        print(": target not found\n", BIOS_GRAY);
    }
    else {
        if (flags == 0) {
            // Hardlink
            // Assuming ln hardlink will only copy file data, refuse folder target
            if (file_read[0] != NULL) {
                write(file_read, linkname, &returncode, target_dir);
                getDirectoryTable(dirtable);
            }
            else {
                print("ln: error: ", BIOS_WHITE);
                print(target, BIOS_WHITE);
                print(" is a folder\n", BIOS_WHITE);
                returncode = -1;
            }
            // FIXME : Extra, weird behavior on folder
            // FIXME : Extra, if sectors entry actually empty, it will be indistinguishable with empty entry
            //       ^ Change empty sectors bytes with something else on writeFile()
        }
        else {
            // Softlink
            // Assuming softlink creating "shortcut"
            i = 0;
            while (i < 2 && valid_filename) {
                j = 0;
                while (j < SECTOR_SIZE && valid_filename) {
                    // Needed buffer because entry may ignoring null terminator
                    clear(filename_buffer, 16);
                    strcpybounded(filename_buffer, dirtable+i*SECTOR_SIZE+j+PATHNAME_BYTE_OFFSET, 14);
                    // Checking entry byte flag ("S" byte)
                    if (dirtable[i*SECTOR_SIZE+j+ENTRY_BYTE_OFFSET] == EMPTY_FILES_ENTRY && !empty_entry_found) {
                        f_entry_sector_idx = i;
                        f_entry_idx = j;
                        empty_entry_found = true;
                    }
                    // Getting entry byte flag of target
                    if (!strcmp(target, filename_buffer) && dirtable[i*SECTOR_SIZE+j+PARENT_BYTE_OFFSET] == target_dir) {
                        target_entry_byte = dirtable[i*SECTOR_SIZE+j+ENTRY_BYTE_OFFSET];
                        f_target_found = true;
                    }
                    // Checking existing filename in same parent folder
                    if (dirtable[i*SECTOR_SIZE+j+PARENT_BYTE_OFFSET] == target_dir) {
                        if (!strcmp(linkname, filename_buffer))
                            valid_filename = false;
                    }
                    j += FILES_ENTRY_SIZE;
                }
                i++;
            }

            if (valid_filename && f_target_found && empty_entry_found) {
                print("ln: ", BIOS_GRAY);
                print(linkname, BIOS_GRAY);
                print(" softlink created\n", BIOS_GRAY);
                dirtable[f_entry_sector_idx*SECTOR_SIZE+f_entry_idx+PARENT_BYTE_OFFSET] = target_dir;
                dirtable[f_entry_sector_idx*SECTOR_SIZE+f_entry_idx+ENTRY_BYTE_OFFSET] = target_entry_byte;
                rawstrcpy((dirtable+f_entry_sector_idx*SECTOR_SIZE+f_entry_idx+PATHNAME_BYTE_OFFSET), linkname);
                // Update files filesystem in memory (dirtable) and write to disk
                i = 0;
                while (i < FILES_SECTOR_SIZE) {
                    directSectorWrite(dirtable+SECTOR_SIZE*i, FILES_SECTOR + i);
                    i++;
                }
            }
            else {
                print("ln: softlink error\n", BIOS_GRAY);
                returncode = -1;
            }
        }

        if (returncode == 0) {
            print(linkname, BIOS_GRAY);
            print(": link created\n", BIOS_GRAY);
        }
        else
            print("ln: file writing error\n", BIOS_GRAY);
    }
}
// TODO : Extra, ln will have problem if target dir and link dir is not equal

// TODO : Extra, Extra, Extra relative pathing mkdir
void mkdir(char *foldername, char current_dir_index) {
    int returncode;
    write(FOLDER, foldername, &returncode, current_dir_index);
    switch (returncode) {
        case 0:
            // Do nothing
            break;
        case -1:
            print("mkdir: ", BIOS_WHITE);
            print(foldername, BIOS_WHITE);
            print(" exist\n", BIOS_WHITE);
            break;
        default:
            print("Usage : mkdir <name>\n");
    }
}

// TODO : Extra, Other misc command (mkdir, rm, etc), ... or redirection

void shell() {
    // char as string / char
    char commands_history[MAX_HISTORY][BUFFER_SIZE]; // "FILO" data type for commands
    char directory_string[BUFFER_SIZE];
    char arg_vector[ARGC_MAX][ARG_LENGTH];
    char directory_table[2][SECTOR_SIZE];
    // char as 1 byte integer
    char io_buffer[SECTOR_SIZE];
    char current_dir_index = ROOT_PARENT_FOLDER;
    char is_between_quote_mark = false;
    char dbg[FILE_SIZE_MAXIMUM]; // DEBUG
    int temp, returncode;
    int i = 0, j = 0, k = 0, argc = 0;

    getDirectoryTable(directory_table);

    clear(commands_history, BUFFER_SIZE*MAX_HISTORY);

    while (true) {
        clear(arg_vector, ARGC_MAX*ARG_LENGTH);
        clear(directory_string, BUFFER_SIZE);
        print("mangga", BIOS_GREEN);
        print(":", BIOS_GRAY);
        directoryStringBuilder(directory_string, directory_table, current_dir_index);
        print(directory_string, BIOS_LIGHT_BLUE);
        print("$ ", BIOS_GRAY);
        shellInput(commands_history, directory_table, current_dir_index);

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


        // Command evaluation
        if (!strcmp("ls", arg_vector[0]))  {
            if (argc == 1)
                ls(directory_table, current_dir_index);
            else if (argc > 1) {
                temp = directoryEvaluator(directory_table, arg_vector[1], &returncode, current_dir_index);
                if (returncode == 0)
                    ls(directory_table, temp);
                else
                    print("ls: path not found\n", BIOS_WHITE);
            }
            else
                print("Usage : ls\n", BIOS_WHITE);
        }
        else if (!strcmp("cat", arg_vector[0])) {
            if (argc == 2)
                cat(directory_table, arg_vector[1], current_dir_index);
            else
                print("Usage : cat <filename>\n", BIOS_WHITE);
        }
        else if (!strcmp("ln", arg_vector[0])) {
            if (argc >= 3) {
                if (!strcmp("-s", arg_vector[1]))
                    ln(directory_table, current_dir_index, 1, arg_vector[2], arg_vector[3]);
                else
                    ln(directory_table, current_dir_index, 0, arg_vector[1], arg_vector[2]);
            }
            else
                print("Usage : ln [-s] <target> <linkname>\n", BIOS_WHITE);
        }
        else if (!strcmp("cd", arg_vector[0])) {
            if (argc == 2)
                current_dir_index = cd(directory_table, arg_vector[1], current_dir_index);
            else
                print("Usage : cd <path>\n", BIOS_WHITE);
        }
        else if (!strcmp("mkdir", arg_vector[0])) {
            if (argc == 2) {
                mkdir(arg_vector[1], current_dir_index);
                getDirectoryTable(directory_table);
            }
            else
                print("Usage : mkdir <name>\n", BIOS_WHITE);
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
        else if (!strcmp("", arg_vector[0])) {
            // WARNING : Multiple space in single block will count as multiple argument due to argsplit above
            // FIXME : Extra, ^ fix this argsplitter
            // Empty string -> doing nothing
        }
        else {
            print(arg_vector[0], BIOS_WHITE);
            print(": command not found\n", BIOS_WHITE);
        }
    }

}
