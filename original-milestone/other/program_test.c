extern int interrupt(int number, int AX, int BX, int CX, int DX);

int main() {
    interrupt(0x21, 0x0, "Test", 0, 1);
}
