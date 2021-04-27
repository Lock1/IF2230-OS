#include "kernel-header/config.h"
#include "std-header/std_stringio.h"
#include "shell-header/shell_common.h"
#include "basic-header/std_opr.h"

void ls(char *dirtable, char target_dir);

int main() {
    char directory_table[FILES_SECTOR_SIZE*SECTOR_SIZE];
    char shell_cache[SECTOR_SIZE];
    char temp;
    int returncode = -1, target_directory;
    char dirstr[BUFFER_SIZE];
    char argc = 0;

    clear(shell_cache, SECTOR_SIZE);
    getDirectoryTable(directory_table);
    getShellCache(shell_cache);

    clear(dirstr, BUFFER_SIZE);
    memcpy(dirstr, shell_cache+ARGV_OFFSET, ARG_LENGTH);
    argc = shell_cache[ARGC_OFFSET];

    if (argc == 1 || argc == 2) {
        if (argc == 2) {
            if (!strcmp("--help", dirstr)) {
                print("ls:\n", BIOS_WHITE);
                print("blue  ", BIOS_LIGHT_BLUE);
                print(": folder\n", BIOS_WHITE);
                print("green ", BIOS_LIGHT_GREEN);
                print(": file\n", BIOS_WHITE);
                print("red   ", BIOS_LIGHT_RED);
                print(": hardlink\n", BIOS_WHITE);
                print("cyan  ", BIOS_LIGHT_CYAN);
                print(": softlink\n", BIOS_WHITE);
                returncode = 2;
            }
            else
                target_directory = directoryEvaluator(directory_table, dirstr, &returncode, shell_cache[CURRENT_DIR_CACHE_OFFSET]);
        }
        else {
            target_directory = shell_cache[CURRENT_DIR_CACHE_OFFSET];
            returncode = 0;
        }

        if (returncode == 0)
            ls(directory_table, target_directory);
        else if (returncode == 1)
            print("ls: target is file\n", BIOS_WHITE);
        else if (returncode != 2)
            print("ls: path not found\n", BIOS_WHITE);
    }
    else
        print("Usage : ls [--help, directory]\n", BIOS_WHITE);

    setShellCache(shell_cache);
    shellReturn();
}

void ls(char *dirtable, char target_dir) {
    int i = 0;
    // char as string / char
    char filename_buffer[16];
    // Use char as 1 byte integer
    char parent_byte_entry, entry_byte_entry;
    char print_color;
    char link_byte_entry;
    while (i < FILES_ENTRY_COUNT) {
        if (getKeyboardCursor(0) > 50)
            print("\n", BIOS_WHITE);

        parent_byte_entry = dirtable[FILES_ENTRY_SIZE*i+PARENT_BYTE_OFFSET];
        entry_byte_entry = dirtable[FILES_ENTRY_SIZE*i+ENTRY_BYTE_OFFSET];
        link_byte_entry = dirtable[FILES_ENTRY_SIZE*i+LINK_BYTE_OFFSET];
        if (parent_byte_entry == target_dir && entry_byte_entry != EMPTY_FILES_ENTRY) {
            clear(filename_buffer, 16);
            strcpybounded(filename_buffer, dirtable+FILES_ENTRY_SIZE*i+PATHNAME_BYTE_OFFSET, 14);

            if (entry_byte_entry == FOLDER_ENTRY)
                print_color = BIOS_LIGHT_BLUE;
            else if (link_byte_entry == SOFTLINK_ENTRY)
                print_color = BIOS_LIGHT_CYAN;
            else if (link_byte_entry == HARDLINK_ENTRY)
                print_color = BIOS_LIGHT_RED;
            else
                print_color = BIOS_LIGHT_GREEN;

            if (isCharInString(CHAR_SPACE, filename_buffer)) {
                print("\"", BIOS_GRAY);
                print(filename_buffer, print_color);
                print("\"", BIOS_GRAY);
            }
            else
                print(filename_buffer, print_color);
            print(" ", BIOS_WHITE);
        }
        i++;
    }
    print("\n", BIOS_WHITE);
}
