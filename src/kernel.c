// Kernel C source code

#include "kernel-header/kernel.h"

int main() {
    makeInterrupt21();
    // Digunakan agar kernel tidak restarting
    while (1);
}

void handleInterrupt21(int AX, int BX, int CX, int DX) {
    // Handler interrupt 0x21

    // Pengecekan nilai AX dan pemanggilan servis yang disediakan
    // Servis tersedia
    // AX = 0, BX = string address -> Menuliskan string yang beraddress pada BX ke layar
    // AX = 1, BX = buffer address -> Membaca input keyboard dan memasukkan pada address buffer di BX
    switch (AX) {
        case 0x0:
            printString(BX);
            break;
        case 0x1:
            readString(BX);
            break;
        default:
            printString("Invalid interrupt");
    }
}

void clear(char *buffer, int length) {
    int i;

    // Ulangi pengosongan buffer hingga panjang length telah tercapai
    for (i = 0; i < length; i++)
        buffer[i] = 0x00;
}

// -- TTY based print --
// AL = Char, BH = Page, BL = Color
void printString(char *string) {
    int i = 0, AX = 0;

    // Ulangi proses penulisan karakter hingga ditemukan null terminator
    while (string[i] != 0x00) {
        // Pembentukan register AX
        AX = 0x0E00 | string[i];

        // Menuliskan karakter dengan INT 10h
        interrupt(0x10, AX, 0x000F, 0, 0);

        // Ganti karakter yang dibaca ke karakter selanjutnya
        i++;
    }
}

void readString(char *string) {
    char singleCharBuffer;
    int currentIndex = 0;
    int AXprintChar;

    // Ulangi pembacaan hingga ditemukan karakter '\r' atau carriage return
    // Tombol enter menghasilkan '\r' dari pembacaan INT 16h
    do {
        // Mengambil input menggunakan INT 16h dan memasukkannya ke singleCharBuffer
        singleCharBuffer = interrupt(0x16, 0x0000, 0, 0, 0);

        // Jika karakter yang dibaca bukan '\r' masukkan ke buffer dan tulis karakter ke layar
        if (singleCharBuffer != '\r') {
            // Menuliskan karakter ke layar
            AXprintChar = 0x0E00 | singleCharBuffer;
            interrupt(0x10, AXprintChar, 0x000F, 0, 0);

            // Memasukkan karakter ke buffer bernama string dan mengganti ke karakter sebelahnya
            string[currentIndex] = singleCharBuffer;
            currentIndex++;
        }
        // Jika karakter merupakan '\r', tuliskan pasangan '\r' dan '\n' ke layar
        else {
            // Menuliskan carriage return agar kursor bergerak ke pojok kiri
            AXprintChar = 0x0E00 | '\r';
            interrupt(0x10, AXprintChar, 0x000F, 0, 0);

            // Menuliskan line feed agar kursor bergerak ke bawah
            AXprintChar = 0x0E00 | '\n';
            interrupt(0x10, AXprintChar, 0x000F, 0, 0);
        }

    } while (singleCharBuffer != '\r');
}
