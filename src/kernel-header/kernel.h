// Kernel header

extern void putInMemory(int segment, int address, char character);

void handleInterrupt21(int AX, int BX, int CX, int DX);
void printString(char *string);
void readString(char *string);
void clear(char *buffer, int length); // Fungsi untuk mengisi buffer dengan 0
