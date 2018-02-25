#include <stdio.h>
#include <stdlib.h>

unsigned int string_length(const char *string) {

	unsigned int i = 1;
	while (*string) {
		++i;
		*++string;
	}

	return i;
}

char* string_encrypt(const char *string) {

	unsigned int byte = 0;

	unsigned int length = string_length(string);

	char *encrypted = malloc(sizeof(char) * length);

	while (*string) {
		switch (byte % 3) {
			case 0x00:
				*encrypted = *string ^ 'I';
				break;
			case 0x01:
				*encrypted = *string ^ 'C';
				break;
			case 0x02:
				*encrypted = *string ^ 'E';
				break;
		}
		*++encrypted;
		*++string;
		++byte;
	}

	while (byte != 0) {
		*--encrypted;
		--byte;
	}

	return encrypted;
}

char *encrypt_hex(char *encrypt) {

	unsigned int length = 2 * string_length(encrypt) - 1;

	char *encrypted_hex = malloc(sizeof(char) * length);

	unsigned int byte = 0;

	while (*encrypt) {
		if (byte++ % 2 == 0) {
			*encrypted_hex = (*encrypt & 0xf0) >> 4;
		} else {
			*encrypted_hex = *encrypt & 0x0f;
		}
		*++encrypted_hex;

		if (byte++ % 2 == 0) {
			*encrypted_hex = (*encrypt++ & 0xf0) >> 4;
		} else {
			*encrypted_hex = *encrypt++ & 0x0f;
		}
		*++encrypted_hex;
	}

	*encrypted_hex-- = '\0';

	while (byte > 0) {
		if (*encrypted_hex < 10) {
			*encrypted_hex += 48;
		} else {
			*encrypted_hex += 87;
		}

		*--encrypted_hex;
		--byte;
	}

	return encrypted_hex++;
}

unsigned int hammington_distance(const char *string1, const char *string2) {

	unsigned int h_dist;
	unsigned int xor;
	int i = string_length(string1) - string_length(string2);

	if (i < 0) {
		h_dist = 8 * (-i);
		i = string_length(string1);
	} else {
		h_dist = 8 * i;
		i = string_length(string2);
	}

	while (i > 0) {

		xor = (*string1 ^ *string2);

		while (xor != 0x00) {
			h_dist += (xor & 0x01);
			xor >>= 1;
		}

		--i;
		*++string1;
		*++string2;
	}

	return h_dist;
}

/* The functions depends on a '\0' closing each string */

char *string_copy(const char *string, unsigned int length) {

    if (length > string_length(string)) {
        length = string_length(string);
    }

    unsigned int pointer_position = length;
    char *new_string = malloc(sizeof(char) * length);
    while (length-- > 0) {
        *new_string++ = *string++;
    }

    return new_string - pointer_position;
}

/* get the keysize between low and high with lowest hammingtondistance, base64 text in file */
unsigned int *get_hammingtonkeysize(int fd, unsigned int low_keysize, unsigned int high_keysize) {

    char* text1;
    char* text2;
    unsigned int ham_dist = high_keysize * 8;
    unsigned int key_array[3];
    unsigned array_position = 0;
    while (low_keysize <= high_keysize) {
        if (read(fd, text1, low_keysize) == -1 ||
           read(fd, text2, low_keysize) == -1 ) {
            exit(0);
        }

        /*
         * make text1, text2 ascii
         * from base64
         */

        if (ham_dist > hammington_distance(text1, text2)) {
            ham_dist = hammington_distance(text1, text2);
            if (array_position < 2) {
                key_array[array_position++] = low_keysize;
            } else {
                key_array[0] = key_array[1];
                key_array[1] = key_array[2];
                key_array[2] = low_keysize;
            }
        }

        free(text1);
        free(text2);
        lseek(fd, -(low_keysize * 2), SEEK_CUR);
        ++low_keysize;
    }

    unsigned int *keysize = key_array;
    return keysize;
}
