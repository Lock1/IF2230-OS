#include "kernel-header/config.h"
#include "std-header/std_stringio.h"
#include "std-header/std_fileio.h"
#include "shell-header/shell_common.h"
#include "basic-header/std_opr.h"
#include "std-header/boolean.h"

#define ENTRY_INDEX 0x14 // Set this

int main() {
    int debug_retcode;
    char garbage[SECTOR_SIZE];

    // Recursion test, don't forget to setting ENTRY_INDEX
    clear(garbage, 512);
    strcpy(garbage, "recursion test");
    write(FOLDER, "fold1\0\0\0\0\0\0\0\0\0", &debug_retcode, ROOT_PARENT_FOLDER);
    write(FOLDER, "infold1\0\0\0\0\0\0\0", &debug_retcode, ENTRY_INDEX);
    write(FOLDER, "infold2\0\0\0\0\0\0\0", &debug_retcode, ENTRY_INDEX);
    write(FOLDER, "in2fold1\0\0\0\0\0\0", &debug_retcode, ENTRY_INDEX+1);
    write(garbage, "in2-file1\0\0\0\0\0", &debug_retcode, ENTRY_INDEX+2);
    write(garbage, "in-file1\0\0\0\0\0\0", &debug_retcode, ENTRY_INDEX);
    write(garbage, "in-file2\0\0\0\0\0\0", &debug_retcode, ENTRY_INDEX);
    write(garbage, "in-file3\0\0\0\0\0\0", &debug_retcode, ENTRY_INDEX);

    // Test structure
    // fold1
    // | - in-file1
    // | - in-file2
    // | - in-file3
    // | - infold1
    //   | - in2fold1
    //     | - <Empty dir>
    // | - infold2
    //   | - in2-file1

    print("Created\n", BIOS_WHITE);

    shellReturn();
}
