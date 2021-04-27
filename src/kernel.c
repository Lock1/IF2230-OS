// Kernel C source code
extern void putInMemory(int segment, int address, char character);

int main () {
    putInMemory(0xB000, 0x8000, 'H');
    putInMemory(0xB000, 0x8001, 0xD);
    putInMemory(0xB000, 0x8002, 'a');
    putInMemory(0xB000, 0x8003, 0xD);
    putInMemory(0xB000, 0x8004, 'i');
    putInMemory(0xB000, 0x8005, 0xD);
    while (1);
}

void handleInterrupt21 (int AX, int BX, int CX, int DX) {

}
