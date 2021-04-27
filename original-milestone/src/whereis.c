#include "kernel-header/config.h"
#include "std-header/std_stringio.h"
#include "shell-header/shell_common.h"
#include "basic-header/std_opr.h"
#include "std-header/boolean.h"

void directoryStringBuilder(char *string, char *dirtable, char current_dir);

int main() {
    // char as character
    char filename_buffer[16];
    char absolute_path[256];
    // char as 1 byte integer
    char directory_table[FILES_SECTOR_SIZE*SECTOR_SIZE];
    char shell_cache[SECTOR_SIZE];
    char arg_vector[ARGC_MAX][ARG_LENGTH];
    char argc = 0;
    int current_dir_index;
    int i = 0;
    bool any_match = false;

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
    if (argc == 2) {
        if (!strcmp("--help", arg_vector[0])) {
            print("Utility to find the location of source/binary file of a command and manuals sections for a specified file\n", BIOS_WHITE);
            print("Possible Usage:\n", BIOS_LIGHT_BLUE);
            print("whereis [file_name]\n", BIOS_LIGHT_CYAN);
            print("whereis [folder_name]\n", BIOS_LIGHT_CYAN);
        }
        else {
            i = 0;
            while (i < FILES_ENTRY_COUNT) {
                clear(filename_buffer, 16);
                strcpybounded(filename_buffer, directory_table+i*FILES_ENTRY_SIZE+PATHNAME_BYTE_OFFSET, 14);
                if (!strcmp(filename_buffer, arg_vector[0])) {
                    any_match = true;
                    clear(absolute_path, 256);
                    directoryStringBuilder(absolute_path, directory_table, directory_table[i*FILES_ENTRY_SIZE+PARENT_BYTE_OFFSET]);
                    strapp(absolute_path, "/");
                    strapp(absolute_path, arg_vector[0]);
                    print(absolute_path, BIOS_LIGHT_BLUE);
                    print("\n", BIOS_WHITE);
                }
                i++;
            }

            if (!any_match) {
                print("whereis: ", BIOS_WHITE);
                print(arg_vector[1], BIOS_WHITE);
                print(" not found", BIOS_WHITE);
            }
        }
    }
    else
        print("Usage : whereis <target>\n", BIOS_WHITE);

    setShellCache(shell_cache);
    shellReturn();
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
