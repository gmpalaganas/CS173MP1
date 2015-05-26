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

    processUpload(clientSocket);
     
    close(clientSocket->socketfd);
    close(serverSocket->socketfd);
    destroySocketObject(clientSocket);
    destroySocketObject(serverSocket);

    return 0;
}

boolean processCommand(char* command, socketObject* clientSocket) {
	char* commandCopy = (char*)malloc(sizeof(char)*(strlen(command) + 1));
	strcpy(commandCopy, command);
    char* token = strtok(commandCopy, " \n\r");
    if (strcmp(token, COMMAND_DOWNLOAD) == 0) {
        
    } else if (strcmp(token, COMMAND_UPLOAD) == 0) {
        return processUpload(clientSocket);
    } else if (strcmp(token, COMMAND_DELETE) == 0) {
        
    } else if (strcmp(token, COMMAND_LIST) == 0) {
        return processList(clientSocket);
    } else if (strcmp(token, COMMAND_LIST_SIZE) == 0) {

    } else {
        printf("Unknown command: %s\n", command);
    }

    free(token);
    free(commandCopy);
    return FALSE;
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

boolean processList(socketObject *clientSocket){

    char* cwd = (char *)malloc(sizeof(char) * BUFFER_LENGTH);
    getcwd(cwd, BUFFER_LENGTH);
    FILE *fileToSend = fopen("._RES/files.txt","w");
    writeDirToFile(fileToSend,cwd);
    
	int filefd = open("._RES/files.txt", O_RDONLY);
    int file_size = getFileSize(fileToSend);
	char* s_size = (char*)malloc(sizeof(char) * 20);
    sprintf(s_size,"%d",file_size);
    
    sendMessage(clientSocket->socketfd,s_size);
    getMessage(clientSocket->socketfd,clientSocket->recv_buffer,BUFFER_LENGTH);
    printf("%s\n",clientSocket->recv_buffer);
	sendFile(clientSocket->socketfd, filefd, getFileSize(fileToSend));
    printf("File Sent\n");
    getMessage(clientSocket->socketfd,clientSocket->recv_buffer,BUFFER_LENGTH);
    printf("%s\n",clientSocket->recv_buffer);

    free(s_size);
	
    fclose(file_size);
    
}
