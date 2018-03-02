#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "b64.h"
#include "lib.h"

/* The functions depends on a '\0' closing each string */
unsigned int string_length(const char *string) {

	unsigned int i = 1;
	while (*string) {
		++i;
		*++string;
	}

	return i;
}

/* Score system:
 * 10 points for spaces
 * 2 points for a-z	(small chars more common than large ones in english text)
 * 1 points for A-Z,(!)==33, (.)==46, (?)==63
 */
unsigned int string_score(unsigned char *string) {

	unsigned int score = 0;
	unsigned char *message = string;
	while (*message) {
		if (*message >= 97 && *message <= 122) {
			score += 2;
		} else if (( *message >= 65 && *message <= 90 )  ||
			         *message == 33 || *message == 46    ||
			         *message == 63) {
			++score;
		} else if (*message == 32) {

			/* Check if there is several space after eachother == NOT POSSIBLE IN ENGLISH LANGUAGE*/
			if (*message != *(message - 1)){
					score += 10;
			}
		}
		++message;
	}
	return score;
}

/* if length = NULL, array1 and array2 will be worked upon as if they were strings '\0'.
 * Also, if array1 and array2 differ in size, there will be a hammington_distance calculation
 * on one of the '\0', which might give wrong answers. The excess length on of an array will be
 * added to the hammington distance by multipying it by 8;
 *
 * length = &uint or NULL; 
 *
 * If you pass a length by yourself, if length is greater than the real one of the char arrays,
 * undefined behavior will occur since there is no checking if the function are going out of bounds. 
 */
unsigned int hammington_distance(unsigned char *array1, unsigned char *array2, unsigned int *length) {

	unsigned int h_dist;
	unsigned int xor;
	int i;

	if (length == NULL){
		i = string_length(array1) - string_length(array2);
		if (i < 0) {
			h_dist = 8 * (-i);
			i = string_length(array1);
		} else {
			h_dist = 8 * i;
			i = string_length(array2);
		}
	} else {
		h_dist = 0;
		i = *length;
	}

	while (i > 0) {

		xor = (*array1 ^ *array2);

		while (xor != 0x00) {
			h_dist += (xor & 0x01);
			xor >>= 1;
		}

		--i;
		*++array1;
		*++array2;
	}
	array1 -= *length;
	array2 -= *length;

	return h_dist;
}

/* get the keysize between low and high with lowest hammingtondistance, base64 text in file */
unsigned int *get_hammingtonkeysize(int fd, unsigned int low_keysize, unsigned int high_keysize, unsigned int size_Keypool) {

    unsigned char *text1;
    unsigned char *text2;

    int i;
    int j = 0;
    unsigned int tmp;
    double tmp_d;
    unsigned int ham_dist = 0;

    unsigned int key_array[size_Keypool];
    double norm_array[size_Keypool];
    unsigned int array_position = 0;

    while (low_keysize <= high_keysize) {
    	
    	text1 = malloc(sizeof(char) * low_keysize);
    	text2 = malloc(sizeof(char) * low_keysize);

    	i = 2;
    	while (i-- > 0) {
	        if (read(fd, text1, low_keysize) == -1) {
	        	perror("text1: failed to read");
	            exit(0);
	        }
	
	        if (read(fd, text2, low_keysize) == -1) {
	        	perror("text2: failed to read");
	            exit(0);
	        }
	        ham_dist += hammington_distance(text1, text2, &low_keysize);
    	}
    	i = size_Keypool - 1;

       	printf("key: %u, ham_dist/key: (%u, %u), norm: %f\n", low_keysize, ham_dist, low_keysize, (double) ham_dist/(2 * low_keysize));

       	if (array_position < size_Keypool) {

       		key_array[array_position] = low_keysize;
       		norm_array[array_position] = (double) ham_dist/(2 * low_keysize);

       		while (i > 0) {

       			if (norm_array[i] > norm_array[i-1]) {
       				tmp_d = norm_array[i-1];
       				norm_array[i-1] = norm_array[i];
       				norm_array[i] = tmp_d;

       				tmp = key_array[i-1];
       				key_array[i-1] = key_array[i];
       				key_array[i] = tmp;
       				--i;
       			} else {
       				--i;
       			}
       		}
       		++array_position;

       	} else {
        	while(i >= 0) {

        		if(norm_array[i] > (double) ham_dist/(2 * low_keysize)) {     
	      			while (j < i) {
	      				norm_array[j] = norm_array[j+1];
	      				key_array[j] = key_array[j+1];
        				++j;		
        			}
        			norm_array[i] = (double) ham_dist/(2 * low_keysize);
        			key_array[i] = low_keysize;
        			j = 0;
        			break;
        		}
        		--i;
        	}
        }
        lseek(fd, 0, SEEK_SET);
        ++low_keysize;
        ham_dist = 0;
    }

    free(text1);
    free(text2);
    unsigned int *keysize = malloc(sizeof(char) * size_Keypool);
    i = 0;
    while (i < size_Keypool) {
    	*(keysize + i) = key_array[i];
    	++i;
    }
    return keysize;
}

/* if -1, error */
int file_Rawlength(int fd) {

	int length = 0;
	int sum = 0;
	unsigned char *block = malloc(sizeof(char) * RAW_BLOCK_LENGTH); 
	while ((length = read(fd, block, RAW_BLOCK_LENGTH)) > 0) {
		sum += length;
	}
	free(block);

	lseek(fd, 0, SEEK_SET);

	if (length == -1) {
		sum = -1;
	}
	return sum;
}

/* what if file_length == 0? */
int *file_Amountblocks(unsigned int block_length, int file_length) {

	int *block_pointer = malloc(sizeof(int) * block_length);
	int i = -1;
	int mod = file_length % block_length;
	unsigned int length = block_length;
	while (block_length * ++i < file_length)
		;
	if (mod > 0) {
		while (length-- > 0) {
			if (mod > 0) {
				*block_pointer++ = i;
				--mod;	
			} else {
				*block_pointer++ = (i-1);
			}
		}
	} else {
		while (length-- > 0) {
			*block_pointer++ = i;
		} 
	}
	return(block_pointer - block_length);
}

/* DONT FORGET TO FREE return */
unsigned char *file_BlockRepeatXORscore(int fd, unsigned int key, int *block_pointer) {

	int score = 0;
	unsigned char *high_xor = malloc(sizeof(unsigned char) * key);
	int high_score = 0;

	unsigned char space_checker;
	char* block = malloc(sizeof(char));;
	int amount_blocks = *block_pointer;
	unsigned int i = 0;
	unsigned char xor = 0;
	int meow;

	while (1) {

		meow = 1;
		while (xor < 128) {
			while (amount_blocks-- > 0) {
				
				if (read(fd, block, 1) == -1) {
					perror("block: failed to read");
					exit(0);
				}
				lseek(fd, key - 1, SEEK_CUR);
		
				*block ^= xor;
				if (*block >= 97 && *block <= 122) {
					score += 2;
				} else if (( *block >= 65 && *block <= 90 )  ||
					         *block == 33 || *block == 46    ||
					         *block == 63) {
					++score;

				/* Checks for repeating spaces */
				} else if (*block == 32 && *block != space_checker) {
					score += 10;
				}
				space_checker = *block;
			}
			space_checker = 0;
			if (score >= high_score) {
				high_score = score;
				*high_xor = xor;
			}
			score = 0;
			lseek(fd, i, SEEK_SET);
			amount_blocks = *block_pointer;
			++xor;
		}
		high_score = 0;	
		xor = 0;
		lseek(fd, 1, SEEK_CUR);
		++i;

		if (i < key) {
				amount_blocks = *++block_pointer;
				*++high_xor;
		} else {
			break;
		}
	}
	lseek(fd, 0, SEEK_SET);
	free(block);
	block_pointer -= key - 1;
	return high_xor - key + 1;
}

unsigned char **file_XORRepeatSearch(int fd, unsigned int *key_array, unsigned int size_Keypool) {

	unsigned char **key_XORarray = malloc(sizeof(char *) * size_Keypool);
	int i = 0;
	int *block_pointer;
	int file_length;

	if ((file_length = file_Rawlength(fd)) == -1) {
		printf("file_length == -1");
		exit(0); 
	}

	while (i < size_Keypool) {
		block_pointer = file_Amountblocks(*key_array, file_length);

		*key_XORarray++ = file_BlockRepeatXORscore(fd, *key_array, block_pointer);

		*++key_array;
		++i;
		free(block_pointer);
	}
	key_array -= size_Keypool;
	return key_XORarray - size_Keypool;
}

void *file_freekey_XORarray(unsigned char **key_XORarray, unsigned int size_Keypool) {

	int i = 0;
	while(i < size_Keypool) {
		free(*key_XORarray++);
		++i;
	}
	key_XORarray = key_XORarray - size_Keypool;
	free(key_XORarray);
	return NULL;
}

/* DONT FORGET TO FREE return */
/* replace 30 with file_length */
unsigned char* file_decrypt(int fd, unsigned int key, char *XOR_array, int file_length) {

	int position = 0;
	unsigned char *array_decrypted = malloc(sizeof(char) * (file_length + 1));

	while (position < file_length) {
		read(fd, array_decrypted, 1);
		*array_decrypted = *array_decrypted ^ *(XOR_array + (position % key));
		*++array_decrypted;
		++position;
	}
	*array_decrypted = '\0';
	lseek(fd, 0, SEEK_SET);
	return array_decrypted = array_decrypted - file_length;
}
