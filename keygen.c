#include "keygen.h"

int main(int argc, char *argv[]){
    if (argc < 2)
    {
        printf("You must provide the length of your key\n");
        return EXIT_FAILURE;
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