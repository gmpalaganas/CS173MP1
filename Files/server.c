#include <stdio.h>
#include "network.h"

#define SERVER_PORT 8888
#define BUFFER_LENGTH 256 


int main(){

    int socketfd; //Socket File Descriptor
    int newsocketfd;
    int client_len; //Client address size
    int len; //Len of read or written in read() or write()
    char* buffer; //read-write buffer
    sockaddr_in server_addr, client_addr; //Structure containing Network Addresses

    socketfd = initSocket(TCP);
    
    if(socketfd < 0)
        error("ERROR opening socket");

    initServer(&server_addr, SERVER_PORT);  

    //Binds the socket to the address
    if(bind(socketfd, (sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        error("Error in binding");

    listen(socketfd,5);

    printf("Waiting for client...\n");

    newsocketfd = acceptClient(socketfd,&client_addr);

    if(newsocketfd < 0)
        error("Error on accept");

    printf("Client connected!\n");
     
    buffer = getMessage(newsocketfd,BUFFER_LENGTH);

    printf("Message is: %s", buffer);
   
    sendMessage(newsocketfd, "I got your message");
    
    free(buffer);

    close(newsocketfd);
    close(socketfd);

    return 0;
}

