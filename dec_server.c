#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SERVER_TYPE "dec_server"
#define CLIENT_TYPE "dec_client"

/**
 * @brief Prints error message to stderr
 * 
 * @param msg message to print
 */
void error(const char *msg) {
    perror(msg);
} 

/**
 * @brief Set up the address struct for the server socket
 * 
 * @param address 
 * @param portNumber 
 */
void setupAddressStruct(struct sockaddr_in* address, int portNumber){
    // Clear out the address struct
    memset((char*) address, '\0', sizeof(*address)); 

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);
    // Allow a client at any address to connect to this server
    address->sin_addr.s_addr = INADDR_ANY;
}

/**
 * @brief Finds offset value of character
 * 
 * @param c original value of 
 * @return int offset value for new char
 */
int charToInt (char c){
	if (c == ' ') return 26;
	else return (c - 'A');
}

/**
 * @brief Converts offset value to real value
 * 
 * @param i offset value
 * @return char ASCII value of converted char
 */
char intToChar(int i){
	if (i == 26) return ' ';
	else return (i + 'A');
}

/**
 * @brief decodes message using encryption key
 * 
 * @param message message to decode
 * @param key encryption key
 */
void decode(char* message, char* key){
    for (int i=0; i < strlen(message); i++){
        char n = charToInt(message[i]) - charToInt(key[i]);
        if (n<0) n += 27;
        message[i] = intToChar(n);
    }
}

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

int main(int argc, char *argv[]){
    int connectionSocket, charsRead;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);

    // Check usage & args
    if (argc < 2) { 
        fprintf(stderr,"USAGE: %s port\n", argv[0]); 
        exit(1);
    } 
  
    // Create the socket that will listen for connections
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        error("ERROR opening socket");
        exit(1);
    }

    // Set up the address struct for the server socket
    setupAddressStruct(&serverAddress, atoi(argv[1]));

    // Associate the socket to the port
    if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
        error("ERROR on binding");
        exit(1);
    }

    // Start listening for connetions. Allow up to 5 connections to queue up
    listen(listenSocket, 5); 

    // Accept a connection, blocking if one is not available until one connects
    while(1){
        // Accept the connection request which creates a connection socket
        connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
        if (connectionSocket < 0){
            error("ERROR on accept");
        }

        // Check if connecting client is dec_client
        char client_type[20];
        recv(connectionSocket, client_type, 20, 0);
        send(connectionSocket, SERVER_TYPE, 19, 0);
        fflush(stdout);

        // If correct client
        if (strcmp(client_type, CLIENT_TYPE) == 0){
            pid_t spawnpid = -5;
            spawnpid = fork();

            switch (spawnpid){
                // If fork() fails
                case -1: {
                    perror("fork() failed!\n");
                    exit(1);
                    break;
                }
                // If process is child
                case 0: {
                    char buffer[200000];
                    char message[200000];
                    char type[10];
                    char key[200000];

                    memset(buffer, '\0', sizeof(buffer));

                    // Read the client's message from the socket
                    int totalRead = 0;
                    while (strstr(buffer, "\n") == NULL) {
                        charsRead = recv(connectionSocket, &buffer[totalRead], 1000, 0); 
                        if (charsRead < 0){
                            error("ERROR reading from socket");
                            break;
                        }
                        totalRead += charsRead;
                    }

                    fflush(stdout);
                    buffer[strcspn(buffer, "\n")] = '\0'; 

                    // Token type, key, and message
                    char *saveptr;
                    char *token = strtok_r(buffer, ",", &saveptr);
                    strcpy(type, token);

                    token = strtok_r(NULL, ",", &saveptr);
                    strcpy(key, token);
                    
                    token = strtok_r(NULL, ",", &saveptr);
                    strcpy(message, token);

                    decode(message, key);
                    fflush(stdout);
                    strcat(message, "\n");

                    fflush(stdout);

                    // Send decodded message
                    int totalSent = 0, charsWritten = 0;
                    while(totalSent < strlen(message)) {
                        charsWritten = send(connectionSocket, &message[totalSent], 1000, 0); 
                        if (charsWritten < 0){
                            error("CLIENT: ERROR writing to socket");
                            break;
                        }
                        totalSent += charsWritten;
                    }

                    if (totalSent < strlen(message)){
                        error("CLIENT: WARNING: Not all data written to socket!\n");
                    }

                    fflush(stdout);
                    close(connectionSocket);
                    exit(0);
                    break;
                } // If parent
                default:
                    break;
            }
        }
        
        close(connectionSocket); 
    }

    close(listenSocket); 
    return 0;
}
