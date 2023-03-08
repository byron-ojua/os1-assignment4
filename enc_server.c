#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SERVER_TYPE "enc_server"
#define CLIENT_TYPE "enc_client"

// Error function used for reporting issues
void error(const char *msg) {
    perror(msg);
} 

// Set up the address struct for the server socket
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

        // ---------------- Add custom code here ---------------------

        // printf("SERVER: Reading client_type\n");
        // fflush(stdout);
        char client_type[20];
        recv(connectionSocket, client_type, 20, 0);
        // printf("SERVER: client_type read\n");
        // fflush(stdout);

        // printf("SERVER: Sending server_type\n");
        // fflush(stdout);
        send(connectionSocket, SERVER_TYPE, 19, 0);
        // printf("SERVER: Sent server_type\n");
        // fflush(stdout);

        // printf("SERVER: client_type is %s\n", client_type);
        fflush(stdout);

        if (strcmp(client_type, CLIENT_TYPE) == 0){
            // printf("SERVER: client_type okay\n");
            fflush(stdout);
            // printf("SERVER: Connected to client running at host %d port %d\n", ntohs(clientAddress.sin_addr.s_addr), ntohs(clientAddress.sin_port));
            fflush(stdout);

            // ----------- FORK ---------------
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
                    // printf("SERVER: Reading\n");
                    int totalRead = 0;
                    while (strstr(buffer, "\n") == NULL) {
                        charsRead = recv(connectionSocket, &buffer[totalRead], 1000, 0); 
                        if (charsRead < 0){
                            error("ERROR reading from socket");
                            break;
                        }
                        totalRead += charsRead;
                    }

                    // printf("SERVER: Recived\n");

                    fflush(stdout);
                    buffer[strcspn(buffer, "\n")] = '\0'; 

                    // printf("SERVER: I received this from the client: \"%s\"\n", buffer);

                    // Token type, key, and message
                    char *saveptr;
                    char *token = strtok_r(buffer, ",", &saveptr);
                    strcpy(type, token);

                    token = strtok_r(NULL, ",", &saveptr);
                    strcpy(key, token);
                    
                    token = strtok_r(NULL, ",", &saveptr);
                    strcpy(message, token);
                    // printf("Type: %s\nKey: %s\nMessage: %s\n", type, key, message);
                    // fflush(stdout);

                    // printf("SERVER: Encoding\n");
                    encrypt(message, key);
                    // printf("ENCMSG: %s\n\n", message);
                    fflush(stdout);
                    // printf("SERVER: Encoded\n");
                    strcat(message, "\n");

                    // fflush(stdout);

                    // printf("SERVER: Sending\n");
                    fflush(stdout);

                    int totalSent = 0, charsWritten = 0;
                    while(totalSent < strlen(message)) {
                        charsWritten = send(connectionSocket, &message[totalSent], 1000, 0); 
                        if (charsWritten < 0){
                            error("CLIENT: ERROR writing to socket");
                            break;
                        }
                        totalSent += charsWritten;
                    }
                    // printf("SERVER: Sent\n");
                    fflush(stdout);

                    close(connectionSocket);
                    exit(0);
                    break;
                }
                default:
                    break;
            }
        }
        
        // Close the connection socket for this client
        close(connectionSocket); 
    }

    // ---------------- End custom code here ----------------------
    // Close the listening socket
    close(listenSocket); 
    return 0;
}
