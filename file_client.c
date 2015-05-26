#include<stdio.h>
#include<stdlib.h>
#include "network.h"
#include "utilities.h"

#define SERVER_PORT 8888
#define BUFFER_LENGTH 256
#define LOCAL_HOST "127.0.0.1"
#define SERVER_ADDRESS "Insert Address Here"

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef int boolean;

void runClientProgram(socketObject* clientSocket);
void displayInstructions();
boolean processCommand(char* command, socketObject* clientSocket);
boolean isAlphaNumeric(char* input);
boolean checkFilename(char* input);
boolean processDownload(char* filename, socketObject* clientSocket);
boolean processUpload(char* filename, socketObject* clientSocket);
boolean processDelete(char* filename);
boolean processList(boolean giveSize);


int main(int argc, char** argv) {
	
    socketObject* clientSocket;
    int filefd; //File Descriptor
    
    FILE* file;
    //Structure containing Network Addresses

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
    clientSocket->server_addr = (sockaddr_in*)malloc(sizeof(sockaddr_in));
    bzero((char *)(clientSocket->server_addr), sizeof(*(clientSocket->server_addr))); //Zero all the bytes
    clientSocket->server_addr->sin_family = AF_INET; //Set to IPv4 IP address family
    clientSocket->server_addr->sin_port = htons(SERVER_PORT); //Converts from host byte order to network byte order
    clientSocket->recv_buffer = (char *)malloc(sizeof(char) * BUFFER_LENGTH);
    clientSocket->send_buffer = (char *)malloc(sizeof(char) * BUFFER_LENGTH);
    
    
    //Copy the address of the server into the server_addr socket structure
    if (inet_pton(AF_INET, argv[1], &((clientSocket->server_addr)->sin_addr)) <= 0) {
        printf("\nError: Could not bind IP address of server.\n");
        return 1;
    }

    printf("Connecting to server...\n");

    while(TRUE) {
        runClientProgram(clientSocket);
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
    free(buffer);*/
    
    free(clientSocket->recv_buffer);
    free(clientSocket->send_buffer);
    close(clientSocket->socketfd); 
    return 0;
}

void runClientProgram(socketObject* clientSocket) {
    char* command;
    boolean isCommandValid = TRUE;
    
    

    do {
        displayInstructions();
        command = getInput(command);
        if (!(isCommandValid = processCommand(command, clientSocket))) printf("Invalid Command.\n");
    } while (!isCommandValid);
    
    free(command);
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

boolean processCommand(char* command, socketObject* clientSocket) {
	char* commandCopy = (char*)malloc(sizeof(char)*(strlen(command) + 1));
	strcpy(commandCopy, command);
    char* token = strtok(commandCopy, " \n\r");
    if (strcmp(token, COMMAND_DOWNLOAD) == 0) {
        token = strtok(NULL, " \n\r");
        if (isFilenameValid(token)) {
            return processDownload(token, clientSocket);
        } else {
            printf("Invalid file name: %s\n", token);
        }
    } else if (strcmp(token, COMMAND_UPLOAD) == 0) {
        token = strtok(NULL, " \n\r");
        if (isFilenameValid(token)) {
            return processUpload(token, clientSocket);
        } else {
            printf("Invalid file name: %s\n", token);
        }
    } else if (strcmp(token, COMMAND_DELETE) == 0) {
        token = strtok(NULL, " \n\r");
        if (isFilenameValid(token)) {
            return processDelete(token);
        } else {
            printf("Invalid file name: %s\n", token);
        }
    } else if (strcmp(token, COMMAND_LIST) == 0) {

    } else if (strcmp(token, COMMAND_LIST_SIZE) == 0) {

    } else {
        printf("Unknown command: %s\n", command);
    }
    free(token);
    free(commandCopy);
    return FALSE;
}

boolean processDownload(char* filename, socketObject* clientSocket) {
    
    return TRUE;
}

boolean processUpload(char* filename, socketObject* clientSocket) {
	if (!connectToServer(clientSocket)) return FALSE;
	
	//Send the request + the filename to the server
	//int socketfd = clientSocket->socketfd;
	
    sendMessage(clientSocket->socketfd, filename);

	//sendMessage(clientSocket->socketfd, filename);
	
	
	//Open the file and prepare for sending
	FILE* fileToSend = fopen(filename, "r");
	int filefd = open(filename, O_RDONLY);
    int file_size = getFileSize(fileToSend);
	char* s_size = (char*)malloc(sizeof(char) * 20);
    sprintf(s_size,"%d",file_size);
    
    sendMessage(clientSocket->socketfd,s_size);
    getMessage(clientSocket->socketfd,clientSocket->recv_buffer,BUFFER_LENGTH);
	sendFile(clientSocket->socketfd, filefd, getFileSize(fileToSend));

    free(s_size);
	
    return TRUE;
}

boolean processDelete(char* filename) {
    return FALSE;
}

boolean connectToServer(socketObject* clientSocket) {
	printf("Connecting to server...");
    if(connect(clientSocket->socketfd, (sockaddr *)(clientSocket->server_addr), sizeof(*(clientSocket->server_addr))) < 0) {
    	error("ERROR: Cannot connect to server.\n");
    	return FALSE;
    }
    printf("Connected!\n");
    return TRUE;
}
