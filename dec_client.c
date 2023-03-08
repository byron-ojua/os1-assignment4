#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

#define SERVER_TYPE "dec_server"
#define CLIENT_TYPE "dec_client"


/**
 * @brief reads the number of chars in a file
 * 
 * @param filename file to check
 * @return long int number of chars in file
 */
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

/**
 * @brief Checks if there are bad characters in the input
 * 
 * @param filename filepath to check
 * @return int 0 false, no bad chars, 1 true, bad chars
 */
int containsBadChars(const char* filename){
	char character;
	FILE* file = fopen(filename, "r");

    while (character == '\0') {
        character = fgetc(file);
        fflush(stdout);

        if (character == ' ' || (character >= 'A' && character <= 'Z' )) {}
        else {
            fclose(file);
	        return 1;
        }
    }
	fclose(file);
	return 0;
}

/**
 * @brief Prints error message to stderr
 * 
 * @param msg message to print
 */
void error(const char *msg) { 
    perror(msg); 
} 

/**
 * @brief Set up the address struct for the client socket
 * 
 * @param address 
 * @param portNumber 
 * @param hostname defualts to localhost
 */
void setupAddressStruct(struct sockaddr_in* address, int portNumber, char* hostname){
    // Clear out the address struct
    memset((char*) address, '\0', sizeof(*address)); 

    // The address should be network capable
    address->sin_family = AF_INET;

    // Store the port number
    address->sin_port = htons(portNumber);

    // Get the DNS entry for this host name
    struct hostent* hostInfo = gethostbyname(hostname); 
    if (hostInfo == NULL) { 
        fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
        exit(0); 
    }
    
    // Copy the first IP address from the DNS entry to sin_addr.s_addr
    memcpy((char*) &address->sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
}

int main(int argc, char *argv[]) {
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    char buffer[200000];
    char message[200000];
    char key[200000];
    
    // Check usage & args
    if (argc < 3) { 
        fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); 
        exit(1); 
    } 

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); 
    if (socketFD < 0){
        error("CLIENT: ERROR opening socket");
        exit(1);
    }

    // Set up the server address struct
    setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        error("CLIENT: ERROR connecting");
        exit(1);
    }

    // Check if connecting server is dec_server
    send(socketFD, CLIENT_TYPE, strlen(CLIENT_TYPE) + 1, 0);
    char conn_server_type[20];
    recv(socketFD, conn_server_type, 19, 0);

    // If not correct server
    if(strcmp(conn_server_type, SERVER_TYPE) != 0){
        char errorMsg[100];
        sprintf(errorMsg, "Error: could not contact dec_server on port %s\n", argv[3]);
        error(errorMsg);
        exit(2);
    } else {
        fflush(stdout);
        memset(buffer, '\0', sizeof(buffer));
        memset(message, '\0', sizeof(message));
        memset(key, '\0', sizeof(key));

        long messagelength = getNumChars(argv[1]);
        long keylength = getNumChars(argv[2]);

        // If key is too short
        if(messagelength > keylength){ 
            char errorMsg[100];
            sprintf(errorMsg, "dec_client: Error: Key '%s' is too short!\n", argv[2]);
            error(errorMsg);
            exit(1);
        // Check input message for bad chars
        } else if (containsBadChars(argv[1])){
            error("dec_client error: input message contains bad characters\n");
            exit(1);
        // Check key for bad chars
        } else if (containsBadChars(argv[2])){
            error("dec_client error: input key contains bad characters\n");
            exit(1);
        } else {
            FILE *inputFile = fopen(argv[1], "r");
            FILE *keyFile = fopen(argv[2], "r");

            // Get input from the user, trunc to buffer - 1 chars, leaving \0
            fgets(message, sizeof(message) - 1, inputFile);
            fgets(key, sizeof(key) - 1, keyFile);

            // Remove the trailing \n that fgets adds
            key[strcspn(key, "\n")] = '\0'; 
            message[strcspn(message, "\n")] = '\0'; 
            sprintf(buffer, "ENC,%s,%s\n", key, message);
            fflush(stdout);

            // Loop through buffer, sending 1000 characters to server at a time
            int totalSent = 0;
            while(totalSent < strlen(buffer)) {
                charsWritten = send(socketFD, &buffer[totalSent], 1000, 0); 
                if (charsWritten < 0){
                    error("CLIENT: ERROR writing to socket");
                    break;
                }
                totalSent += charsWritten;
            }

            if (totalSent < strlen(buffer)){
                error("CLIENT: WARNING: Not all data written to socket!\n");
            }

            // Clear out the buffer again for reuse
            memset(buffer, '\0', sizeof(buffer));
            fflush(stdout);

            // Read data from the socket, leaving \0 at end
            int totalRead = 0;
            while(strstr(buffer, "\n") == NULL) {
                charsRead = recv(socketFD, &buffer[totalRead], 1000, 0); 
                if (charsRead < 0){
                    error("CLIENT: ERROR reading from socket");
                    break;
                }
                totalRead += charsRead;
            }

            fflush(stdout);
            printf("%s", buffer);
            fflush(stdout);
        }
    }

    close(socketFD); 
    return 0;
}