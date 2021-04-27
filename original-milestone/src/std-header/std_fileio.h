// 13519214 - Standard function header

// ---------------- Standard Macro ----------------
#define NULL 0x00



extern int interrupt(int number, int AX, int BX, int CX, int DX);


// --- File I/O ---
void write(char *buffer, char *path, int *returncode, char parentIndex);
// Write "buffer" with name "path" at "parentIndex" folder

void read(char *buffer, char *path, int *returncode, char parentIndex);
// Read file with name "path" at "parentIndex" folder and write to "buffer"

void directSectorWrite(char *buffer, int sector);
// Direct writing to sector, no checking

void directSectorRead(char *buffer, int sector);
// Direct reading to sector, no checking

// --- Misc ---
void memcpy(char *dest, char *src, int bytes);
// Copying data from src to desc until bytes reached

void remove(char *filename, int *returncode, char parentIndex);
// Deleting file or folder
