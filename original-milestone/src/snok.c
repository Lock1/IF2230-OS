#include "kernel-header/config.h"
#include "std-header/std_stringio.h"
#include "shell-header/shell_common.h"
#include "basic-header/std_opr.h"
#include "std-header/boolean.h"

#define CTRL_C_SCANCODE_LOW 0x03
#define CTRL_C_SCANCODE_HIGH 0x2E

void snok();

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
    if (argc == 1)
        snok();
    else
        print("Usage : snok\n", BIOS_WHITE);

    setShellCache(shell_cache);
    shellReturn();
}


void snok() {
    // char as 1 byte integer
    char c, scancode;
    int i = 0, rawKey;
    int last_direction = SCANCODE_DOWN_ARROW;
    int savedCursorRow = getKeyboardCursor(true);
    int savedCursorCol = getKeyboardCursor(false);
    int p_x, p_y;
    int j = 0;

    // Sad that getFullKeyPress() is blocking :( and no kernel size left
    clearEntireScreen();
    hideKeyboardCursor();
    setKeyboardCursor(10, 10);
    putchar('o');
    setKeyboardCursor(10, 10);
    savedCursorRow = getKeyboardCursor(true);
    savedCursorCol = getKeyboardCursor(false);
    p_x = savedCursorRow;
    p_y = savedCursorCol;
    i = 0;
    do {
        savedCursorRow = getKeyboardCursor(true);
        savedCursorCol = getKeyboardCursor(false);

        switch (scancode) {
            case SCANCODE_DOWN_ARROW:
                if (p_x < 25) {
                    p_x++;
                    // putchar(' ');
                    setKeyboardCursor(savedCursorRow + 1, savedCursorCol);
                    putchar('o');
                    setKeyboardCursor(savedCursorRow + 1, savedCursorCol);
                }
                break;
            case SCANCODE_UP_ARROW:
                if (p_x > 0) {
                    p_x--;
                    // putchar(' ');
                    setKeyboardCursor(savedCursorRow - 1, savedCursorCol);
                    putchar('o');
                    setKeyboardCursor(savedCursorRow - 1, savedCursorCol);
                }
                break;
            case SCANCODE_RIGHT_ARROW:
                if (p_y < 80) {
                    p_y++;
                    // putchar(' ');
                    setKeyboardCursor(savedCursorRow, savedCursorCol + 1);
                    putchar('o');
                    setKeyboardCursor(savedCursorRow, savedCursorCol + 1);
                }
                break;
            case SCANCODE_LEFT_ARROW:
                if (p_y > 0) {
                    p_y--;
                    // putchar(' ');
                    setKeyboardCursor(savedCursorRow, savedCursorCol - 1);
                    putchar('o');
                    setKeyboardCursor(savedCursorRow, savedCursorCol - 1);
                }
                break;
        }
        savedCursorRow = getKeyboardCursor(true);
        savedCursorCol = getKeyboardCursor(false);



        rawKey = getFullKey();
        c = rawKey & 0xFF;      // AL Value
        scancode = rawKey >> 8; // AH Value
        // WARNING : Prioritizing ASCII before scancode
        if (!(scancode == CTRL_C_SCANCODE_HIGH && c == CTRL_C_SCANCODE_LOW)) {
            // If char (AL) is not ASCII Control codes, check scancode (AH)
            switch (scancode) {
                case SCANCODE_DOWN_ARROW:
                case SCANCODE_RIGHT_ARROW:
                case SCANCODE_UP_ARROW:
                case SCANCODE_LEFT_ARROW:
                    last_direction = scancode;
                    break;
                default:
                    break;
            }
        }
    } while (!(scancode == CTRL_C_SCANCODE_HIGH && c == CTRL_C_SCANCODE_LOW));

    clearEntireScreen();
    setKeyboardCursor(0, 0);
    print("snok: exit\n", BIOS_WHITE);
}
