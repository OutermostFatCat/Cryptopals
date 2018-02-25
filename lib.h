char* string_encrypt(const char *string);
unsigned int string_length(const char *string);
char *string_copy(const char *string, unsigned int length);

char *encrypt_hex(char *encrypt);

unsigned int hammington_distance(char *string1, char *string2);
unsigned int get_hammingtonkeysize(int fd, unsigned int low_keysize, unsigned int high_keysize);

int open(const char *path, int oflag, /* mode_t mode */ );
off_t lseek(int fd, off_t offset, int whence);
