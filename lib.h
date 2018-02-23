char* string_encrypt(const char *string);
unsigned int string_length(const char *string);

char *encrypt_hex(char *encrypt);

unsigned int hammington_distance(char *string1, char *string2);

int open(const char *path, int oflag, ... /* mode_t mode */ );