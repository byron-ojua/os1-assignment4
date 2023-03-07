#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// Error function used for reporting issues
void error(const char *msg) { 
    perror(msg); 
    exit(1); 
} 

int main(int argc, char *argv[]){
    if (argc < 2){
        error("You must provide the length of your key");
    }
    srand(time(NULL));

    int keylen = atoi(argv[1]);
    // char key[keylen + 1];

    for(int i = 0; i < keylen; i++){
        int charCode = rand() % 27;

        if (charCode == 26){
            printf("%c", 32);
        } else {
            printf("%c", charCode + 65);
        }
    }

    printf("\n");
    return EXIT_SUCCESS;
}