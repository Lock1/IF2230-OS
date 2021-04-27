// 13519214 - Kernel header

#include "config.h"

// INT 21H Handler
// TODO : Extra, Add

// --- Kernel Behavior ---
// ~ Kernel will assume exists external procedure called shell()
//          and will spawning shell after kernel setup complete.

// Implemented in assembly
extern void putInMemory(int segment, int address, char character);
extern void makeInterrupt21();
extern int interrupt(int number, int AX, int BX, int CX, int DX);
extern void launchProgram(int segment);

// External procedure call
extern void shell();

void handleInterrupt21(int AX, int BX, int CX, int DX);
// Interupt 21H Handler

void printString(char *string);
// System call for string print

void readString(char *string);
// System call for string input

void readSector(char *buffer, int sector);
// Reading file at sector and copy to buffer

void writeSector(char *buffer, int sector);
// Writing buffer at sector

void readFile(char *buffer, char *path, int *result, char parentIndex);
// Read file with relative path
// If type is folder, return NULL char at position "buffer"
// -- Error code list --
// -1 - File not found
// -- readFile() services --
// buffer is pointer to location which will be used for reading file
// path is filename, only maximum 14 char
// result will be used as error code container
// parentIndex used as "P" byte value / parent index at files filesystem entry

void writeFile(char *buffer, char *path, int *sectors, char parentIndex);
// Writing file with relative path
// WARNING : writeFile() will treat buffer as stream of binary
// WARNING : writeFile() will stop if and only if writeFile() found 5 consecutive null terminator
// -- Error code list --
// 0 - Exit successfully
// -1 - File exists
// -2 - Not enough entry in files filesystem
// -3 - Not enough empty sectors
// -4 - Invalid folder
// -- writeFile() services --
// buffer is file entry to be written
// path is filename, only maximum 14 char
// sectors will be used as error code container
// parentIndex used as "P" byte value / parent index at files filesystem entry
// If buffer == NULL, creating folder instead file

void deleteFile(char *path, int *returncode, char parentIndex);
// Delete file with relative path
// -- Error code list --
// 1 - Exit successfully, deleted object is folder
// 0 - Exit successfully, deleted object is file
// -1 - File not found
// -- deleteFile() services --
// path is filename, only maximum 14 char
// parentIndex used as "P" byte value / parent index at files filesystem entry
// Will delete both file and folder

void executeProgram(char *filename, int segment, int *success, char parentIndex);
// Executing program from file
