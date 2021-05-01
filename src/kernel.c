// Kernel C source code
extern void putInMemory(int segment, int address, char character);

void handleInterrupt21(int AX, int BX, int CX, int DX);
void printString(char *string);
void readString(char *string);
void clear(char *buffer, int length); //Fungsi untuk mengisi buffer dengan 0

int main() {
    makeInterrupt21();
    while (1);
}

void handleInterrupt21(int AX, int BX, int CX, int DX) {
    switch (AX) {
        case 0x0:
            printString(BX);
            break;
        case 0x1:
            // readString(BX);
            break;
        default:
            printString("Invalid interrupt");
    }
}

void clear(char *buffer, int length) {
    int i;
    for (i = 0; i < length; i++)
        buffer[i] = 0x00;
}

// -- TTY based print --
// AL = Char, BH = Page, BL = Color
void printString(char *string) {
    int i = 0, AX = 0;
    while (string[i] != 0x00) {
        AX = 0x0E00 | string[i];
        interrupt(0x10, AX, 0x000F, 0, 0);
        i++;
    }
}
