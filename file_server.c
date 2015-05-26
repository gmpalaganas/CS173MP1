#include <stdio.h>
#include "network.h"
#include "utilities.h"

#define SERVER_PORT 8888
#define BUFFER_LENGTH 256 

typedef int boolean;

int main(){
    socketObject *serverSocket, *clientSocket;
    int client_len; //Client address size
    int len; //Len of read or written in read() or write()

    /*initSocketObject(serverSocket);*/
    /*initSocketObject(clientSocket);*/

    serverSocket = (socketObject*)malloc(sizeof(socketObject));
    serverSocket->addr = (sockaddr_in*)malloc(sizeof(sockaddr_in));
    serverSocket->send_buffer = (char*)malloc(sizeof(char));
    serverSocket->recv_buffer = (char*)malloc(sizeof(char));

    clientSocket = (socketObject*)malloc(sizeof(socketObject));
    clientSocket->addr = (sockaddr_in*)malloc(sizeof(sockaddr_in));
    clientSocket->send_buffer = (char*)malloc(sizeof(char));
    clientSocket->recv_buffer = (char*)malloc(sizeof(char));

    serverSocket->socketfd = initSocket(TCP);
    
    if(serverSocket->socketfd < 0)
        error("ERROR opening socket");

    initServer(serverSocket->addr, SERVER_PORT);  

    //Binds the socket to the address
    if(bind(serverSocket->socketfd, (sockaddr *) serverSocket->addr, sizeof(*serverSocket->addr)) < 0)
        error("Error in binding");

    listen(serverSocket->socketfd,5);

    printf("Waiting for client...\n");

    clientSocket->socketfd = acceptClient(serverSocket->socketfd,clientSocket->addr);

    if(clientSocket->socketfd < 0)
        error("Error on accept");

    printf("Client connected!\n");

    /*getMessage(clientSocket->socketfd,clientSocket->recv_buffer,BUFFER_LENGTH);*/
    /*sendACK(clientSocket->socketfd);*/
    /*printf("COMMAND: %s\n", clientSocket->recv_buffer);*/

    //Start server program
    processCommand(clientSocket);
     
    close(clientSocket->socketfd);
    close(serverSocket->socketfd);
    destroySocketObject(clientSocket);
    destroySocketObject(serverSocket);

    return 0;
}

boolean processCommand(socketObject* clientSocket) {
	getMessage(clientSocket->socketfd, clientSocket->recv_buffer, BUFFER_LENGTH);
    printf("Command: |%s\n", clientSocket->recv_buffer);
    sendACK(clientSocket->socketfd);
    if (strcmp(clientSocket->recv_buffer, COMMAND_DOWNLOAD) == 0) {
        return processDownload(clientSocket);
    } else if (strcmp(clientSocket->recv_buffer, COMMAND_UPLOAD) == 0) {
        return processUpload(clientSocket);
    } else if (strcmp(clientSocket->recv_buffer, COMMAND_DELETE) == 0) {
        return processDelete(clientSocket);
    } else if (strcmp(clientSocket->recv_buffer, COMMAND_LIST) == 0) {
        return processList(FALSE,clientSocket);
    } else if (strcmp(clientSocket->recv_buffer, COMMAND_LIST_SIZE) == 0) {
        return processList(TRUE,clientSocket);
    } else {
        printf("Unknown clientSocket->recv_buffer: %s\n", clientSocket->recv_buffer);
    }

    free(clientSocket->recv_buffer);
    return FALSE;
}

boolean processDownload(socketObject* clientSocket) {
	
	FILE* file;
	
	//Get the name of the file and send the ACK
	getMessage(clientSocket->socketfd, clientSocket->recv_buffer, BUFFER_LENGTH);
	char* filename = (char*)malloc(sizeof(char)*(strlen(clientSocket->recv_buffer) + 1));
	strcpy(filename, clientSocket->recv_buffer);
	printf("File Name: %s\n", clientSocket->recv_buffer);
	sendACK(clientSocket->socketfd);
	
	//Send the size of the file. If the file does not exist, send -1.
	file = fopen(filename, "r");
	char* s_size = (char*)malloc(sizeof(char) * 20);
	int file_size;
	if (file == NULL) {
		file_size = -1;
		sprintf(s_size, "%d", file_size);
		
		//Send the -1 size (since it's an error) and get the ACK
		sendMessage(clientSocket->socketfd, s_size);
		getMessage(clientSocket->socketfd, clientSocket->recv_buffer, BUFFER_LENGTH);
		return;
	} else {
		//Send the size of the file and get the ACK
		file_size = getFileSize(file);
		sprintf(s_size, "%d", file_size);
		sendMessage(clientSocket->socketfd, s_size);
		getMessage(clientSocket->socketfd, clientSocket->recv_buffer, BUFFER_LENGTH);
		
		//Send the file and get the ACK
		int filefd = open(filename, O_RDONLY);
		sendFile(clientSocket->socketfd, filefd, file_size);
		printf("File Sent.\n");
		getMessage(clientSocket->socketfd, clientSocket->recv_buffer, BUFFER_LENGTH);

	}
	
	fclose(file);
	free(filename);
	free(s_size);

	return TRUE;	

}

boolean processUpload(socketObject *clientSocket){

    FILE* file;

    getMessage(clientSocket->socketfd,clientSocket->recv_buffer,BUFFER_LENGTH);
    printf("File Name: %s\n",clientSocket->recv_buffer);
    sendACK(clientSocket->socketfd);
    
    file = fopen(clientSocket->recv_buffer,"w");
    getMessage(clientSocket->socketfd,clientSocket->recv_buffer,BUFFER_LENGTH);

    int size = atoi(clientSocket->recv_buffer);
    printf("File Size: %s\n", clientSocket->recv_buffer);

    sendACK(clientSocket->socketfd);
    getFile(clientSocket->socketfd,file,size);
    sendACK(clientSocket->socketfd);
    
    fclose(file);

    return TRUE;

}

boolean processList(boolean giveSize,socketObject *clientSocket){

    char* cwd = (char *)malloc(sizeof(char) * BUFFER_LENGTH);
    getcwd(cwd, BUFFER_LENGTH);
    char* fileName = (giveSize == TRUE)?"._RES/files_size.txt":"._RES/files.txt";
    FILE *fileToSend = fopen(fileName,"w");

    writeDirToFile(fileToSend,cwd,giveSize);
    
	int file_size;
    file_size = getFileSize(fileToSend);

	fclose(fileToSend);

    fileToSend = fopen(fileName,"r");
	char* s_size = (char*)malloc(sizeof(char) * 20);

    //Send the size of the file and get the ACK
    sprintf(s_size, "%d", file_size);
    sendMessage(clientSocket->socketfd, s_size);
    getMessage(clientSocket->socketfd, clientSocket->recv_buffer, BUFFER_LENGTH);

    //Send the file and get the ACK
    int filefd = open(fileName, O_RDONLY);
    sendFile(clientSocket->socketfd, filefd, file_size);
    getMessage(clientSocket->socketfd, clientSocket->recv_buffer, BUFFER_LENGTH);
	

	fclose(fileToSend);
	free(s_size);

}

boolean processDelete(socketObject *clientSocket) {
	FILE* file;
	
	//Get the file name and send an ACK
	getMessage(clientSocket->socketfd, clientSocket->recv_buffer, BUFFER_LENGTH);
	char* filename = (char*)malloc(sizeof(char)*strlen(clientSocket->recv_buffer));
	strcpy(filename, clientSocket->recv_buffer);
	sendACK(clientSocket->socketfd);
	
	//Determine the error code or result code to send to the client.
	int resultCode;
	file = fopen(filename, "r");
	if (file == NULL) {
		resultCode = 0;
	} else {
		if (remove(filename) == 0) resultCode = 1;
		else resultCode = 2;
	}
	char* s_resultCode = (char*)malloc(sizeof(char)*5);
	sprintf(s_resultCode, "%d", resultCode);
	
	//Send the result code and get an ACK
	sendMessage(clientSocket->socketfd, s_resultCode);
	getMessage(clientSocket->socketfd, clientSocket->recv_buffer, BUFFER_LENGTH);
	
	if (file != NULL) fclose(file);
	free(s_resultCode);
	
	return TRUE;
}


