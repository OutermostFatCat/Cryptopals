#include "lib.h"

int main()
{	
	int fd;
	unsigned int *key_array;
	unsigned int low_key = 2;
	unsigned int high_key = 40;
	unsigned char **key_XORarray;

	/* For some reason, program stops working if size_Keypool >= 7 */
	unsigned int size_Keypool = 6;

	if ((fd = open("file.raw", O_RDONLY)) == -1) {
		printf("couldn't open file");
		exit(0);
	}

	/* Get an array of three best keys between low and high */
	key_array = get_hammingtonkeysize(fd, low_key, high_key, size_Keypool);

	/* find most probable repeated XORs for each key */
	key_XORarray = file_XORRepeatSearch(fd, key_array, size_Keypool);

	int i = 0;		
	unsigned char *tmp;
	unsigned char *decrypted_message;
	unsigned int score = 0;
	while (i < size_Keypool) {
		tmp = file_decrypt(fd, *(key_array + i), *(key_XORarray + i), file_Rawlength(fd));
		if (string_score(tmp) > score) {
			decrypted_message = tmp;
			score = string_score(tmp);
		} else {
			free(tmp);
		}	
		++i;
	}

	printf("%s\n", decrypted_message);
	free(decrypted_message);
	free(key_array);
	key_XORarray = file_freekey_XORarray(key_XORarray, size_Keypool);
	close(fd);
	return 0;
}
