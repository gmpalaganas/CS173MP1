#include <stdio.h>
#include <pthread.h>
#include "network.h"
#include "utilities.h"

#define SERVER_PORT 8888
#define BUFFER_LENGTH 256 
#define NUM_CLIENTS 3

typedef int boolean;
pthread_mutex_t clientRequestLock;
pthread_mutex_t listLock;

void *runClientHandler(void *socket) {
	printf("Client handle created.\n");
	int i;
	socketObject* clientSocket = (socketObject*) socket;
	printf("Client connected with ID: %d\n", clientSocket->ID);
	
	pthread_mutex_lock(&clientRequestLock);
	processCommand(clientSocket);
	pthread_mutex_unlock(&clientRequestLock);
	
	close(clientSocket->socketfd);
	//destroySocketObject(clientSocket);
	
	//clientSocket = (socketObject*)malloc(sizeof(socketObject));
	//clientSocket->addr = (sockaddr_in*)malloc(sizeof(sockaddr_in));
	//clientSocket->send_buffer = (char*)malloc(sizeof(char));
   	//clientSocket->recv_buffer = (char*)malloc(sizeof(char));
   	clientSocket->ID = -1;

	pthread_exit(NULL);
}

int main(){
    socketObject *serverSocket, *clientSocket;
    int client_len; //Client address size
    int len; //Len of read or written in read() or write()

    /*initSocketObject(serverSocket);*/
    /*initSocketObject(clientSocket);*/
    pthread_mutex_init(&clientRequestLock, NULL);
    pthread_mutex_init(&listLock, NULL);

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
        error("ERROR: Could not open server socket.\n");

    initServer(serverSocket->addr, SERVER_PORT);  

    //Binds the socket to the address
    if(bind(serverSocket->socketfd, (sockaddr *) serverSocket->addr, sizeof(*serverSocket->addr)) < 0)
        error("ERROR: Could not bind server to address.\n");

	socketObject** clientSet = (socketObject**)malloc(sizeof(socketObject*)*NUM_CLIENTS);
	int i;
	for (i = 0; i < NUM_CLIENTS; i++) {
		clientSet[i] = (socketObject*)malloc(sizeof(socketObject));
		(clientSet[i])->addr = (sockaddr_in*)malloc(sizeof(sockaddr_in));
		(clientSet[i])->send_buffer = (char*)malloc(sizeof(char));
    	(clientSet[i])->recv_buffer = (char*)malloc(sizeof(char));
    	(clientSet[i])->ID = -1;
	}
	
	listen(serverSocket->socketfd, NUM_CLIENTS);

    printf("Waiting for clients...\n");
	//Prepare threads
	pthread_t threads[NUM_CLIENTS];
	int rc;
	
	int nextOpen;
	while (TRUE) {
		nextOpen = searchClient(clientSet);
		(clientSet[nextOpen])->socketfd = acceptClient(serverSocket->socketfd, clientSocket->addr);
    	if ((clientSet[nextOpen])->socketfd < 0) error("ERROR: Could not establish connection with a client.\n");
    	else {
    		clientSet[nextOpen]->ID = nextOpen;
    		rc = pthread_create(&threads[nextOpen], NULL, runClientHandler, (void *)clientSet[nextOpen]);
    		if (rc) {
    			printf("ERROR: Could not create a handle for client connection.\n");
    			close(clientSet[nextOpen]);
    			(clientSet[nextOpen])->ID = -1;
    		}
    	}
    }
     
    close(serverSocket->socketfd);    
	destroySocketObject(serverSocket);

	pthread_exit(NULL);
    //return 0;
}

//Searches for the first available client socket position
int searchClient(socketObject** clientSet) {
	while (TRUE) {
		int i;
		for (i = 0; i < NUM_CLIENTS; i++) {
			if ((clientSet[i])->ID == -1) return i;
			printf("Client[%d] ID: %d\n", i, (clientSet[i])->ID);
		}
	}
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

    strcat(cwd,"/Files");
    printf("Directory: %s\n", cwd);
    printf("RES Fiels: %s\n", fileName);
    writeDirToFile(fileToSend,cwd,giveSize);
    printf("I got here");
    
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
	

    free(cwd);
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


