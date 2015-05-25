#include<stdio.h>
#include<stdlib.h>
#include "network.h"

#define SERVER_PORT 8888
#define BUFFER_LENGTH 256
#define LOCAL_HOST "127.0.0.1"
#define SERVER_ADDRESS "Insert Address Here"

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef int boolean;

void getUserChoice();
void displayInstructions();
boolean processCommand(char* input);
char* getInput();
boolean isAlphaNumeric(char* input);
boolean checkFilename(char* input);
boolean processDownload(char* filename);
boolean processUpload(char* filename);
boolean processDelete(char* filename);
boolean processList(boolean giveSize);


int main(int argc, char** argv) {

    socketObject* clientSocket;
    int filefd; //File Descriptor
    int len; //Len of read or written in read() or write()    
    char file_name[BUFFER_LENGTH];
    char sendBuffer[BUFFER_LENGTH];
    
    FILE* file;
    sockaddr_in* server_addr; //Structure containing Network Addresses

    if (argc != 2) {
        printf("\nPlease input the IP Address of the server.\n");
        printf("Format: %s <ip_address>", argv[0]);
        return 1;
    }
    
    //Create the client socket
    clientSocket = (socketObject*) malloc(sizeof(clientSocket));
    if ((clientSocket->socketfd = initSocket(TCP)) < 0) {
        printf("\nError: Could not initialize client socket.\n");
        return 1;
    }

    //Setup the server address
    bzero((char *) server_addr, sizeof(*server_addr));

    server_addr->sin_family = AF_INET;
    //htons converts from host byte order to network byte order
    server_addr->sin_port = htons(SERVER_PORT);

    //Copies the address of the server into the server_addr socket structure
    if (inet_pton(AF_INET, argv[1], &(server_addr->sin_addr)) <= 0) {
        printf("\nError: Could not bind IP address of server.\n");
        return 1;
    }

    printf("Connecting to server...\n");

    while(TRUE) {
        getUserChoice();
        break;
    }

    /*
    if(connect(socketfd, (sockaddr *) &server_addr, sizeof(server_addr)) < 0) 
        error("ERROR connecting");

    printf("Connected to server!\n");

    printf("Enter File Name: ");
    scanf("%s%*c",file_name);

    sendMessage(socketfd, file_name);

    file = fopen(file_name,"r");
    int size = getFileSize(file);
    filefd = open(file_name, O_RDONLY);
    char s_size[15];
    sprintf(s_size,"%d",size);
    printf("File Name: %s\nFile Size: %d\n",file_name,size);
    sendMessage(socketfd,s_size);

    sendFile(socketfd,filefd,size);

    int wait = 1;

    fclose(file);
    /*free(buffer);*/
    
    close(clientSocket->socketfd);
    return 0;
}

void getUserChoice() {
    char* command;

    displayInstructions();
    if (!checkCommand(command = getInput())) printf("Invalid command.\n");
    else {

    }
}

void displayInstructions() {
    printf("\nMAIN MENU\nInput the command in the given format.\n\n");
    printf("List files on the server - LIST\n");
    printf("List files on the server with the file sizes - LIST_SIZE\n");
    printf("Download a file - DOWNLOAD <file name with extension>\n");
    printf("Upload a file - UPLOAD <file name with extension>\n");
    printf("Delete a file (from server) - DELETE <file name with extension>\n");
    printf("Input Command: ");
}

boolean checkCommand(char* input) {
    char* token = strtok(input, " \n\r");
    if (strcmp(token, COMMAND_DOWNLOAD) == 0) {
        token = strtok(NULL, " \n\r");
        if (isFilenameValid(token)) {
            return processDownload(token);
        } else {
            printf("Invalid file name: %s\n", token);
            return FALSE;
        }
    } else if (strcmp(token, COMMAND_UPLOAD) == 0) {
        token = strtok(NULL, " \n\r");
        if (isFilenameValid(token)) {
            return processUpload(token);
        } else {
            printf("Invalid file name: %s\n", token);
            return FALSE;
        }
    } else if (strcmp(token, COMMAND_DELETE) == 0) {
        token = strtok(NULL, " \n\r");
        if (isFilenameValid(token)) {
            return processDelete(token);
        } else {
            printf("Invalid file name: %s\n", token);
            return FALSE;
        }
    } else if (strcmp(token, COMMAND_LIST) == 0) {

    } else if (strcmp(token, COMMAND_LIST_SIZE) == 0) {

    } else {
        printf("Unknown command: %s\n", input);
        return FALSE;
    }
    return TRUE;
}

boolean processDownload(char* filename) {
    return FALSE;
}

boolean processUpload(char* filename) {
    return FALSE;
}

boolean processDelete(char* filename) {
    return FALSE;
}


/* Utility Functions */
char* getInput() {
    char* input = (char*) malloc(sizeof(char));
    char nextChar = '\n';
    int ctr;
    for (ctr = 0; (nextChar = getchar()) != '\n' && nextChar != EOF; ctr++) {
        input = (char*)realloc(input, (ctr + 1)*sizeof(char));
        input[ctr] = nextChar;
    }
    input[ctr] = '\0';
    return input;
}

boolean isAlphaNumeric(char* input) {
    int i, len = strlen(input);
    for (i = 0; i < len; i++) {
        if (!isalnum(input[i])) return FALSE;
    }
    return TRUE;
}

boolean isFilenameValid(char* input) {
    return isAlphaNumeric(strtok(input, ".")) && isAlphaNumeric(strtok(NULL, " \n\r"));
}