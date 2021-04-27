#include "kernel-header/config.h"
#include "std-header/std_stringio.h"
#include "shell-header/shell_common.h"
#include "basic-header/std_opr.h"
#include "std-header/boolean.h"

#define PRINTF_BUFFER 1024

bool isNumber(char *buffer);

int main() {
    char directory_table[FILES_SECTOR_SIZE*SECTOR_SIZE];
    char shell_cache[SECTOR_SIZE];
    char arg_vector[ARGC_MAX][ARG_LENGTH];
    char buffer[PRINTF_BUFFER];
    char move_buffer[PRINTF_BUFFER];
    char number_buffer[32];
    char argc = 0;
    bool is_error = false;
    int current_dir_index;
    int i = 0;
    int current_variadic_arg = 1;

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
    if (argc >= 3) {
        clear(buffer, PRINTF_BUFFER);
        strcpy(buffer, arg_vector[0]);

        while (i < PRINTF_BUFFER-1 && buffer[i] != CHAR_NULL) {
            if (buffer[i] == '%' && buffer[i+1] == 's') {
                clear(move_buffer, PRINTF_BUFFER);
                strcpy(move_buffer, buffer+i+2);
                buffer[i] = CHAR_NULL;
                strapp(buffer, arg_vector[current_variadic_arg]);
                strapp(buffer, move_buffer);
                current_variadic_arg++;
            }
            else if (buffer[i] == '%' && buffer[i+1] == 'd') {
                if (isNumber(arg_vector[current_variadic_arg])) {
                    clear(move_buffer, PRINTF_BUFFER);
                    strcpy(move_buffer, buffer+i+2);
                    buffer[i] = CHAR_NULL;
                    strapp(buffer, arg_vector[current_variadic_arg]);
                    strapp(buffer, move_buffer);
                    current_variadic_arg++;
                }
                else {
                    print("printf: error ", BIOS_WHITE);
                    print(arg_vector[current_variadic_arg], BIOS_WHITE);
                    print(" not a number\n", BIOS_WHITE);
                    is_error = true;
                }
            }
            i++;
        }

        if (!is_error) {
            print(buffer, BIOS_WHITE);
            print("\n", BIOS_WHITE);
        }
    }
    else
        print("Usage : printf <format> <argument>\n", BIOS_WHITE);

    setShellCache(shell_cache);
    shellReturn();
}

bool isNumber(char *buffer) {
    int i = 0;

    while (buffer[i] != '\0') {
        if (!('0' <= buffer[i] && buffer[i] <= '9'))
            return false;
        i++;
    }
    return true;
}
