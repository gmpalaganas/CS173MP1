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
    int filefd; //File Descriptor
    int len; //Len of read or written in read() or write()
    char* buffer; //read-write buffer
    char file_name[BUFFER_LENGTH];
    char sendBuffer[BUFFER_LENGTH];
    FILE* file;
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
    close(socketfd);
    return 0;
}

