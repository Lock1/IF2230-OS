// 13519214 - I/O Functions

#include "kernel-header/output.h"

void getFullKeyWrapper(int *ptr) {
    *ptr = getFullKeyPress();
}
