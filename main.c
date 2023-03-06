#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define CHAR_BUFFER 100000

int charToInt (char c){
	if (c == ' ') return 26;
	else return (c - 'A');
}

char intToChar(int i){
	if (i == 26) return ' ';
	else return (i + 'A');
}

void encrypt(char* message, char* key){
	for (int i=0; i < strlen(message); i++){
        char n = (charToInt(message[i]) + charToInt(key[i])) % 27;

        message[i] = intToChar(n);
	}
}

void decode(char* message, char* key){
    for (int i=0; i < strlen(message); i++){
        char n = charToInt(message[i]) - charToInt(key[i]);
        if (n<0){
            n += 27;
        }
        message[i] = intToChar(n);
    }
}

long int getNumChars(const char* filename){
	int character;
	long int count = 0;
	FILE* file = fopen(filename, "r");

    while (1) {
        character = fgetc(file);

        if (character == EOF || character == '\n') break;

        count++;
    }
	fclose(file);
	return count;
}


int main(int argc, char *argv[]){
	long filelength = getNumChars(argv[1]);
	long keylength = getNumChars(argv[2]);

	if(filelength > keylength){ //if file is greater than key
		printf("Key is too short!\n");
		// error("Key is too short!\n");
	} else {
        FILE *msgPath = fopen(argv[1], "r");
        FILE *keyPath = fopen(argv[2], "r");
        char str[CHAR_BUFFER];
        char key[CHAR_BUFFER];
        fgets(str, sizeof(str), msgPath);
        fgets(key, sizeof(key), keyPath);

        str[strcspn(str, "\n")] = '\0';
        key[strcspn(key, "\n")] = '\0';

        // printf("STR is %s\n", str);
        printf("KEY: %s\n", key);

        encrypt(str, key);
        printf("ENCRYPTING: %s\n", str);

        decode(str, key);
        printf("DECODING: %s\n", str);
    }

    return EXIT_SUCCESS;
}