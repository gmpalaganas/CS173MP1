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
boolean processDelete(char* filename, socketObject* clientSocket);
boolean processList(boolean giveSize,socketObject* clientSocket);


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
    /*initSocketObject(clientSocket);*/

    clientSocket = (socketObject*)malloc(sizeof(socketObject));
    clientSocket->addr = (sockaddr_in*)malloc(sizeof(sockaddr_in));
    clientSocket->send_buffer = (char*)malloc(sizeof(char));
    clientSocket->recv_buffer = (char*)malloc(sizeof(char));

    if ((clientSocket->socketfd = initSocket(TCP)) < 0) {
        printf("\nError: Could not initialize client socket.\n");
        return 1;
    }

    //Setup the server address
    bzero((char *)(clientSocket->addr), sizeof(*(clientSocket->addr))); //Zero all the bytes
    clientSocket->addr->sin_family = AF_INET; //Set to IPv4 IP address family
    clientSocket->addr->sin_port = htons(SERVER_PORT); //Converts from host byte order to network byte order
        
    //Copy the address of the server into the addr socket structure
    if (inet_pton(AF_INET, argv[1], &((clientSocket->addr)->sin_addr)) <= 0) {
        printf("\nError: Could not bind IP address of server.\n");
        return 1;
    }

    printf("Connecting to server...\n");

	runClientProgram(clientSocket);

    close(clientSocket->socketfd); 
    destroySocketObject(clientSocket);
    return 0;
}

void runClientProgram(socketObject* clientSocket) {
    char* command;
    boolean isCommandValid = TRUE;
    
    do {
        displayInstructions();
        command = getInput(command);
        if (!(isCommandValid = processCommand(command, clientSocket))) {
        	printf("Invalid Command.\n");
        }
        if (recv(clientSocket->socketfd, clientSocket->recv_buffer, 1, MSG_PEEK) >= 0) close(clientSocket->socketfd);
        clientSocket->socketfd = initSocket(TCP);
    } while (strcmp(command,COMMAND_EXIT) != 0);
    
    free(command);
}

void displayInstructions() {
    printf("\nMAIN MENU\nInput the command in the given format.\n\n");
    printf("List files on the server - LIST\n");
    printf("List files on the server with the file sizes - LIST_SIZE\n");
    printf("Download a file - DOWNLOAD <file name with extension>\n");
    printf("Upload a file - UPLOAD <file name with extension>\n");
    printf("Delete a file (from server) - DELETE <file name with extension>\n");
    printf("Exit the client - EXIT\n");
    printf("Input Command: ");
}

boolean processCommand(char* command, socketObject* clientSocket) {
	char* commandCopy = (char*)malloc(sizeof(char)*(strlen(command) + 1));
	strcpy(commandCopy, command);
    char* token = strtok(commandCopy, " \n\r");
    char* filename;
    if (strcmp(token, COMMAND_DOWNLOAD) == 0) {
        token = strtok(NULL, "\n\r");
        filename = (char*)malloc(sizeof(char)*strlen(token));
        trimWhiteSpaceBefore(&filename);
        if (isFilenameValid(filename)) {
            return processDownload(token, clientSocket);
        } else {
            printf("Invalid file name: %s\n", token);
        }
    } else if (strcmp(token, COMMAND_UPLOAD) == 0) {
        token = strtok(NULL, "\n\r");
        filename = (char*)malloc(sizeof(char)*strlen(token));
        trimWhiteSpaceBefore(&filename);
        if (isFilenameValid(filename)) {
            return processUpload(token, clientSocket);
        } else {
            printf("Invalid file name: %s\n", token);
        }
    } else if (strcmp(token, COMMAND_DELETE) == 0) {
        token = strtok(NULL, "\n\r");
        filename = (char*)malloc(sizeof(char)*strlen(token));
        trimWhiteSpaceBefore(&filename);
        if (isFilenameValid(filename)) {
            return processDelete(token, clientSocket);
        } else {
            printf("Invalid file name: %s\n", token);
        }
    } else if (strcmp(token, COMMAND_LIST) == 0) {
            return processList(FALSE,clientSocket);
    } else if (strcmp(token, COMMAND_LIST_SIZE) == 0) {
            return processList(TRUE,clientSocket);
    } else if (strcmp(token, COMMAND_EXIT) == 0) {
        printf("Exiting the client. Thank you!\n");
        return TRUE;

    } else {
    	printf("Unknown command: %s\n", command);
    }

    free(commandCopy);
    return FALSE;
}

boolean processDownload(char* filename, socketObject* clientSocket) {
    if (!connectToServer(clientSocket)) return FALSE;

	//Send the command and get the ACK
    sendMessage(clientSocket->socketfd, COMMAND_DOWNLOAD);
    getMessage(clientSocket->socketfd, clientSocket->recv_buffer, BUFFER_LENGTH);
    
    char* f_name = (char *)malloc(sizeof(char) * BUFFER_LENGTH);
    strcpy(f_name,"Files/");
    strcat(f_name,filename);
    
    //Send the filename and get the ACK
    sendMessage(clientSocket->socketfd, f_name);
    getMessage(clientSocket->socketfd, clientSocket->recv_buffer, BUFFER_LENGTH);
       
    //Prepare the file for download
    FILE* downloadedFile = fopen(f_name, "w");
    
    //Get the size of the file and send an ACK
    getMessage(clientSocket->socketfd, clientSocket->recv_buffer, BUFFER_LENGTH);
    sendACK(clientSocket->socketfd);
    int size = atoi(clientSocket->recv_buffer);
    printf("File Size: %s\n", clientSocket->recv_buffer);
	
	if (size < 0) {
		printf("ERROR: File cannot be found on server.\n");
		return FALSE;
	}
    
    //Get the file and send an ACK
    getFile(clientSocket->socketfd, downloadedFile, size, TRUE);
    sendACK(clientSocket->socketfd);
    
    free(f_name);
    fclose(downloadedFile);
    
    printf("File download complete!\n");
    
    return TRUE;
}

boolean processUpload(char* filename, socketObject* clientSocket) {
	
	
	char* f_name = (char *)malloc(sizeof(char) * BUFFER_LENGTH);
    strcpy(f_name,"Files/");
    strcat(f_name,filename);
	
	printf("File to send: %s\n", f_name);
	FILE* fileToSend = fopen(f_name, "r");
	if (fileToSend == NULL) {
		printf("ERROR: Could not find file in client.\n");
		return FALSE;
	}

	if (!connectToServer(clientSocket)) return FALSE;
		
	//Send the request + the filename to the server
	//int socketfd = clientSocket->socketfd;
    
    sendMessage(clientSocket->socketfd, COMMAND_UPLOAD);
    getMessage(clientSocket->socketfd,clientSocket->recv_buffer,BUFFER_LENGTH);

    sendMessage(clientSocket->socketfd, f_name);
    getMessage(clientSocket->socketfd,clientSocket->recv_buffer,BUFFER_LENGTH);

	//Open the file and prepare for sending
	int filefd = open(f_name, O_RDONLY);
    int file_size = getFileSize(fileToSend);
	char* s_size = (char*)malloc(sizeof(char) * 20);
    sprintf(s_size,"%d",file_size);
    
    sendMessage(clientSocket->socketfd,s_size);
    getMessage(clientSocket->socketfd,clientSocket->recv_buffer,BUFFER_LENGTH);
	sendFile(clientSocket->socketfd, filefd, getFileSize(fileToSend));
    printf("File Sent\n");
    getMessage(clientSocket->socketfd,clientSocket->recv_buffer,BUFFER_LENGTH);

    free(s_size);
    free(f_name);
	
    return TRUE;
}

boolean processDelete(char* filename, socketObject* clientSocket) {
    if (!connectToServer(clientSocket)) return FALSE;

	//Send the command and get an ACK
	sendMessage(clientSocket->socketfd, COMMAND_DELETE);
	getMessage(clientSocket->socketfd, clientSocket->recv_buffer, BUFFER_LENGTH);
	
	//Send the filename and get an ACK
    char* f_name = (char *)malloc(sizeof(char) * BUFFER_LENGTH);
    strcpy(f_name,"Files/");
    strcat(f_name,filename);
	sendMessage(clientSocket->socketfd, f_name);
	getMessage(clientSocket->socketfd, clientSocket->recv_buffer, BUFFER_LENGTH);
	
	//Get the confirmation about whether the file was deleted.
	getMessage(clientSocket->socketfd, clientSocket->recv_buffer, BUFFER_LENGTH);
	sendACK(clientSocket->socketfd);
	int result = atoi(clientSocket->recv_buffer);
	switch (result) {
		case 0:
			printf("ERROR: File could not be found on server!\n");
			break;
		case 1:
			printf("File was successfully deleted!\n");
			return TRUE;
		default:
			printf("ERROR: Unknown error occurred while deleting file.\n");
			break;
	}

    free(f_name);
    return FALSE;
}


boolean processList(boolean giveSize,socketObject* clientSocket) {

    if (!connectToServer(clientSocket)) return FALSE;

	//Send the command and get the ACK
    char* command = (giveSize == TRUE)?COMMAND_LIST_SIZE:COMMAND_LIST;
    sendMessage(clientSocket->socketfd, command);
    getMessage(clientSocket->socketfd, clientSocket->recv_buffer, BUFFER_LENGTH);
    
    //Prepare the file for download
    
    char* fileName = (giveSize == TRUE)?"._RES/files_size.txt":"._RES/files.txt";
    FILE* downloadedFile = fopen(fileName, "w");
    
    //Get the size of the file and send an ACK
    getMessage(clientSocket->socketfd, clientSocket->recv_buffer, BUFFER_LENGTH);
    sendACK(clientSocket->socketfd);
    int size = atoi(clientSocket->recv_buffer);

    //Get the file and send an ACK
    getFile(clientSocket->socketfd, downloadedFile, size, FALSE);
    sendACK(clientSocket->socketfd);
    fclose(downloadedFile);
   
    downloadedFile = fopen(fileName, "r");
    printFileContents(downloadedFile);
    fclose(downloadedFile);
    
    return TRUE;
}

boolean connectToServer(socketObject* clientSocket) {

	printf("Connecting to server...");
    if(connect(clientSocket->socketfd, (sockaddr *)(clientSocket->addr), sizeof(*(clientSocket->addr))) < 0) {
    	error("ERROR: Cannot connect to server.\n");
    	return FALSE;
    }
    printf("Connected!\n");
    return TRUE;
}
