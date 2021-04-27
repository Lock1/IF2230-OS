// Commonly used function or procedures in shell
#define CURRENT_DIR_CACHE_OFFSET 0xF
#define HISTORY_CACHE_OFFSET 0x10
#define CACHE_SIGNATURE_OFFSET 0x0
#define ARGC_OFFSET 0xE
#define ARGV_OFFSET 0x150
#define ARGV_2_OFFSET 0x170
#define ARGV_3_OFFSET 0x190

// TODO : Cleanup
#define LS_TARGET_DIR_CACHE_OFFSET 1
#define CD_TARGET_DIR_CACHE_OFFSET 1

#define CACHE_SIGNATURE 'm'

#define ARG_LENGTH 32
#define ARGC_MAX 8
#define BUFFER_SIZE 64
#define MAX_HISTORY 5

#define EXECUTABLE_SIGNATURE "\x55\x89\xE5\x57\x56\x81\xC4"

void getDirectoryTable(char *buffer);
// WARNING : No bound checking
// Get all directory table, put in buffer

void shellReturn();
// Exec shell on segment 0x2000

void getShellCache(char *buffer);
// Get shell cache, used for message passing and state storage

void setShellCache(char *buffer);
// Set shell cache, used for updating cache

char isBinaryFileMagicNumber(char *buffer);
// Checking whether bytes in buffer is a supported executable binary file
// Header samples
// 55 89 E5 57 56 81 C4 BF F9 30 C0 88 86 BB F9 4C
// 55 89 E5 57 56 81 C4 BC F9 B8 00 02 50 8D 9E FC
// 55 89 E5 57 56 81 C4 FC F9 B8 FF FF 89 86 F8 F9
// Taking 55 89 E5 57 56 81 C4 as magic number to compare

char directoryEvaluator(char *dirtable, char *dirstr, int *returncode, char current_dir);
// Directory string evaluator, returning evaluated files filesystem entry index
