// 13519214 - Standard function header

// ---------------- Standard Macro ----------------
#define NULL 0x00



extern int interrupt(int number, int AX, int BX, int CX, int DX);

// ---------------- Standard string operation ----------------
int strlen(char *string);
// Standard string length

void strcpy(char *dest, char *src);
// Standard strcpy without returning

void strcpybounded(char *dest, char *src, int n);
// strcpy only first n characters

void rawstrcpy(char *dest, char *src);
// strcpy without copying null terminator

void rawstrcpybounded(char *dest, char *src, int n);
// strcpy only first n characters and without null terminator

char strcmp(char *s1, char *s2);
// Standard strcmp function

void strrev(char *string);
// Reversing string at pointed location

void strapp(char *string1, char* string2);
// WARNING : No bound checking
// Append string 1 with string 2

void strtobytes(char *buffer, char *string, int bytecount);
// Converting string to bytes with size "bytecount" at "buffer"

void inttostr(char *buffer, int n);
// WARNING : Naive implementation, no bound checking
// Converting integer n to string pointed at buffer

char isCharInString(char c, char *string);
// Finding whether char c is in string

int getLastMatchedCharIdx(char c, char *string);
// Get last index of matching char

int getFirstMatchedCharIdx(char c, char *string);
// Get first index of matching char

void clear(char *buffer, int length);
// Clearing string buffer

// ---------------- Standard I/O ----------------
void print(char *string, char color);
// Simple printing with color

void gets(char *string);
// Simple keyboard input, ye olde gets()

void putchar(char a);
// Standard 1 char output

int getFullKey();
// Getting 1 keypress, blocking, no echo

void showKeyboardCursor();
// Showing keyboard cursor to screen

void hideKeyboardCursor();
// Disable keyboard cursor

void setKeyboardCursor(char r, char c);
// Set keyboard cursor position to row r and column c

int getKeyboardCursor(bool isrow);
// Get current keyboard cursor position

void scrollScreen();
// Scroll entire screen upward 1 row

// --- File I/O ---
void write(char *buffer, char *path, int *returncode, char parentIndex);
// Write "buffer" with name "path" at "parentIndex" folder

void read(char *buffer, char *path, int *returncode, char parentIndex);
// Read file with name "path" at "parentIndex" folder and write to "buffer"

void directSectorWrite(char *buffer, int sector);
// Direct writing to sector, no checking


// --- Misc ---
void memcpy(char *dest, char *src, int bytes);
// Copying data from src to desc until bytes reached
