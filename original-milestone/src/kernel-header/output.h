// 13519214 - Input-output related function
// FIXME : Extra, Bizarre bcc filename behavior, cannot using io.h

// Implemented in assembly
extern int getFullKeyPress();

void getFullKeyWrapper(int *ptr);
// Wrapper for getFullKeyPress, changing ptr value to AH = scancode, AL = ASCII
