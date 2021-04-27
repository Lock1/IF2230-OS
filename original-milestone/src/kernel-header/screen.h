// 13519214 - Other function

#include "../std-header/boolean.h"

// Configuration
#define LOGO_STRING "0000000000000000000011111110000000000000000000n0000000011011111100110111111011000000000000000n0000000111001111110010011110101100000000000000n0000011110110101101111011100010010000000000000n0000110000001101111010000010011100010000000000n0000111011001111111011101101000001001000000000n0000011111111110111100100000001001011100000000n0001110110000011000111011100000000000100000000n0001110111011001110111101000000101101000000000n0000101011111111001111110101010001100000000000n0000111111010011001100001100010000000100000000n0000111110010101000001110101000000010001000000n0000111100011111011001000001000000100001000000n0000011111100100110001111100000000001001000000n0000010100111001110000010000000000000001000000n0000011111111001011011001000110000000000000000n0000001101100110010111001010100100101000100000n0000001111010110010111100001100000000001000000n0000000111100101011101100100000000000101000000n0000000011011111000101000100110011100000000000n0000000001101011111001111000001000000000000000n0000000001110111110010011100110100000000000000n0000000000001100111001111011101000000000000000n0000000000000001111110000010000000000000000000n0000000000000000010010101000000000000000000000n0000000000000000000000000000000000000000000000n";
// TODO : Extra, Use better technique
// Note : VGA Mode 3 offer 25 lines / row and 80 characters


#define LOADING_X_OFFSET 7
#define LOADING_Y_OFFSET 70

#define LOGO_X_OFFSET 8
#define LOGO_Y_OFFSET 0

// Implemented in assembly
extern int getRawCursorPos();


void charVideoMemoryWrite(int offset, char character);
// putInMemory() wrapper for video memory writing

void clearScreen();
// Wipe entire screen

void setCursorPos(int r, int c);
// Move cursor position
// Note : Row and Column start from 0

int getCursorPos(bool isRow);
// Get cursor position

void drawPixel(int x, int y, int color);
// Change color single pixel on screen,
// (0,0) starting from top left corner
// Right increase x, down increase y

void drawRectangle(int x, int y, int w, int h);
// Draw single white rectangle without fill

void bitDraw(int xs, int ys, int color, char *bitarray);
// Draw from string containing image bit

void drawBootLogo();
// Drawing boot logo

void directCharPrint(char a, int color);
// Direct interrupt to printing colored char

void printColoredString(char *string, char color);
// Generalized version of printString

void enableKeyboardCursor();
// Enabling keyboard cursor

void disableKeyboardCursor();
// Disabling keyboard cursor

void getCursorPosWrapper(int *ptr, bool isrow);
// Wrapper for getCursorPos, changing ptr to row or column cursor position

void scrollScreenSingleRow();
// Scrolling screen 1 row up
