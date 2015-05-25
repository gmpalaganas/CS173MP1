#include<stdio.h>
#include<stdlib.h>
#include "network.h"

#define SERVER_PORT 8888
#define BUFFER_LENGTH 256 

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;


int main(){

    int socketfd; //Socket File Descriptor
    int len; //Len of read or written in read() or write()
    char* buffer; //read-write buffer
    char sendBuffer[BUFFER_LENGTH];
    sockaddr_in server_addr; //Structure containing Network Addresses
    hostent *server;
    
    socketfd = initSocket(TCP);

    if(socketfd < 0)
        error("ERROR opening socket");
 
    server = gethostbyname("127.0.0.1");

    if(server == NULL)
        error("No such host\n");

    bzero((char *) &server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
   
    //Copies the address of the server into the server_addr socket structure
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr,server->h_length);
    
    //htons converts from host byte order to network byte order
    server_addr.sin_port = htons(SERVER_PORT);

    printf("Connecting to server...\n");

    if(connect(socketfd, (sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        error("ERROR connecting");


    printf("Connected to server!\n");

    printf("Enter message: ");
    fgets(sendBuffer,BUFFER_LENGTH - 1, stdin);
    
    sendMessage(socketfd,sendBuffer);

    buffer = getMessage(socketfd,BUFFER_LENGTH);
    
    printf("%s\n", buffer);
    
    free(buffer);
    close(socketfd);
    return 0;
}