// 13519214 - Assembly setup
// Compile using bcc and Link with kernel_asm.o
extern int interrupt(int number, int AX, int BX, int CX, int DX);

int main() {
    getCursorPos();
    return 0;
}

void handleInterrupt21(int AX, int BX, int CX, int DX) {

}

int getCursorPos() {
    int DX;
    DX = 3;
    return DX;
}
