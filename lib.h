#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

unsigned int string_length(const char *string);
unsigned int string_score(unsigned char *string);

unsigned int hammington_distance(unsigned char *string1,unsigned char *string2, unsigned int *length);
unsigned int *get_hammingtonkeysize(int fd, unsigned int low_keysize, unsigned int high_keysize, unsigned int size_Keypool);

int file_Rawlength(int fd);
int *file_Amountblocks(unsigned int block_length, int file_length);
unsigned char *file_BlockRepeatXORscore(int fd, unsigned int key, int *block_pointer);
unsigned char **file_XORRepeatSearch(int fd, unsigned int *key_array, unsigned int size_Keypool);
void *file_freekey_XORarray(unsigned char **key_XORarray, unsigned int size_Keypool);
unsigned char *file_decrypt(int fd, unsigned int key, char *XOR_array, int file_length);
#define RAW_BLOCK_LENGTH 10

int open(const char *path, int oflag, ... /* mode_t mode */ );
off_t lseek(int fd, off_t offset, int whence);
#define O_RDONLY 0
