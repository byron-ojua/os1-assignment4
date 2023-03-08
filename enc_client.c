#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

#define SERVER_TYPE "enc_server"
#define CLIENT_TYPE "enc_client"

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
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

// Error function used for reporting issues
void error(const char *msg) { 
    perror(msg); 
} 

// Set up the address struct
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
        exit(0); 
    } 

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); 
    if (socketFD < 0){
        error("CLIENT: ERROR opening socket");
    }

    // Set up the server address struct
    setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        error("CLIENT: ERROR connecting");
    }

        // ----------- Add custom code below here ----------------

    // printf("CLIENT: Sending client_type\n");
    // fflush(stdout);
    send(socketFD, CLIENT_TYPE, strlen(CLIENT_TYPE) + 1, 0);
    // printf("CLIENT: client_type sent\n");
    // fflush(stdout);

    // printf("CLIENT: Reading server_type\n");
    // fflush(stdout);
    char conn_server_type[20];
    recv(socketFD, conn_server_type, 19, 0);
    // printf("CLIENT: Read server_type\n");
    // fflush(stdout);

    // printf("CLIENT: server_type is %s\n", conn_server_type);
    // fflush(stdout);

    if(strcmp(conn_server_type, SERVER_TYPE) != 0){
        char errorMsg[100];
        sprintf(errorMsg, "Error: could not contact enc_server on port %s\n", argv[3]);
        error(errorMsg);
        exit(2);
    } else {
        // Clear out the buffer arrays
        // printf("CLIENT: server_type okay\n");
        fflush(stdout);
        memset(buffer, '\0', sizeof(buffer));
        memset(message, '\0', sizeof(message));
        memset(key, '\0', sizeof(key));

        long messagelength = getNumChars(argv[1]);
        long keylength = getNumChars(argv[2]);

        if(messagelength > keylength){ 
            char errorMsg[100];
            sprintf(errorMsg, "enc_client: Error: Key '%s' is too short!\n", argv[2]);
            error(errorMsg);
            exit(1);
        } else if (containsBadChars(argv[1])){
            // printf("Input contains bad characters\n");
            error("enc_client error: input message contains bad characters\n");
            exit(1);
        } else if (containsBadChars(argv[2])){
            // printf("Key contains bad characters\n");
            error("enc_client error: input key contains bad characters\n");
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

            // printf("CLIENT: Sending\n");
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

            if (charsWritten < strlen(buffer)){
                error("CLIENT: WARNING: Not all data written to socket!\n");
            }

            // printf("CLIENT: Sent\n");
            // printf("totalSent %d\n", totalSent);
            fflush(stdout);

            // Clear out the buffer again for reuse
            memset(buffer, '\0', sizeof(buffer));

            // Read data from the socket, leaving \0 at end
            
            // printf("CLIENT: Reading\n");
            fflush(stdout);
            int totalRead = 0;
            while(strstr(buffer, "\n") == NULL) {
                charsRead = recv(socketFD, &buffer[totalRead], 1000, 0); 
                if (charsRead < 0){
                    error("CLIENT: ERROR reading from socket");
                    break;
                }
                totalRead += charsRead;
            }

            // printf("CLIENT: Read\n");
            fflush(stdout);

            // buffer[strcspn(buffer, "\n")] = '\0'; 
            printf("%s", buffer);

            // printf("CLIENT: Encrypted text is \"%s\"\n", buffer);
            fflush(stdout);
        }
    }

    // ---------------- End custom code here ---------------
    // Close the socket
    close(socketFD); 
    return 0;
}