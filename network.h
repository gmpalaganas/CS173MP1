#include<sys/types.h>
#include<sys/socket.h>
#include<sys/sendfile.h>
#include<netinet/in.h>
#include<netdb.h>
#include<fcntl.h>
#include<string.h>
#include<stdlib.h>

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;

#define TCP SOCK_STREAM
#define UDP SOCK_DGRAM
#define IPv4 AF_INET
#define TRUE 1
#define FALSE 0

#define COMMAND_DOWNLOAD "DOWNLOAD"
#define COMMAND_UPLOAD "UPLOAD"
#define COMMAND_LIST "LIST"
#define COMMAND_LIST_SIZE "LIST_SIZE"
#define COMMAND_DELETE "DELETE"

/*

sockaddr_in Structure

struct sockaddr_in {
    short            sin_family;   // e.g. AF_INET
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
};

struct in_addr {
    unsigned long s_addr;  // load with inet_aton()
};

*/

/*
 * Note on bytes in C:
 *  It seems that bytes in C are represented as strings
 *  rather than integers which might explain why byte
 *  related functions accept char* as parameters
 *
 */

typedef struct socketObject {
    int socketfd;
    int bufferSize;
    char* buffer;
} socketObject;

//Prints error then exits with error code 1
void error(char *msg){
    perror(msg);
    exit(1);
}

/*  GENERAL NETWORKING FUNCTIONS  */

//Creates a socket with domain IPv4 and type of given type
int initSocket(int type){
    //socket(domain, type, protocol) 
    //When protocol is 0 system selects the best for the socket type
    //UDP for DGRAM and TCP for STREAM
    int ret = socket(IPv4, type, 0);
    if(ret < 0)
        error("Error opening socket\n");
    return ret;
}

char* getMessage(int socketfd, const int bufferLen) {
    
    char* buffer = malloc(bufferLen);
    bzero(buffer,bufferLen);
    //read() returns the length of the read message returns -1 if fails
    int len = read(socketfd, buffer, bufferLen);

    if(len < 0)
        error("Error reading message\n");
        
    return buffer; 

}

void getFile(int socketfd, FILE* file) {
    
    char buffer[BUFSIZ];
    bzero(buffer,BUFSIZ);

    //The first data sent is the file size
    int len = read(socketfd, buffer, BUFSIZ);
    int fileSize = atoi(buffer);

    if(len < 0)
        error("Error reading file\n");

    int remainingSize = fileSize; 

    while(((len = read(socketfd, buffer, BUFSIZ)) > 0) && (remainingSize > 0)){
        
        fwrite(buffer, sizeof(char), len, file);
        remainingSize -= len;

        if(len < 0)
            error("Error reading file\n");
        
    }


}


void sendMessage(int socketfd, char* message){
     
    //write() returns the length of the written message returns -1 if fails
    int len = write(socketfd,message,strlen(message));
    
    if(len < 0)
        error("Error sending message\n");

}

void sendFile(int socketfd, int filefd, int size){
     
    //write() returns the length of the written message returns -1 if fails
    int len = 0; 
    int fileSize = size;
    int remainingSize = fileSize;
    off_t offset = 0;
    
    while(((len = sendfile(socketfd,filefd,&offset,BUFSIZ)) > 0) && remainingSize > 0){
        
        if(len < 0)
            error("Error sending file\n");

        remainingSize -= len;

    }


}

int getFileSize(FILE* file){

    int ret = 0;
    fseek(file,0,SEEK_END);
    ret = ftell(file); 
    fseek(file,0,SEEK_SET);

    return ret;
    

}

/* SERVER RELATED FUNCTIONS */

//Initizializes the server sockaddr_in structure
void initServer(sockaddr_in *serverAddr, int port){
    //bzero() sets all values in a buffer to zero
    bzero((char *) serverAddr, sizeof(serverAddr)); 
    serverAddr->sin_family = AF_INET;
    //htons() coverts host byte order of port to network byte order
    serverAddr->sin_port = htons(port);
    //INADDR_ANY sets the address to the address of the machine wherein the code is running 
    serverAddr->sin_addr.s_addr = INADDR_ANY;

}

//Binds server socket
int bindSocket(int socketfd, sockaddr_in *serverAddr){
    
    //bind() returns 0 if success and -1 if fail
    int ret = bind(socketfd, (sockaddr *)serverAddr, sizeof(serverAddr));
    
    if(ret < 0)
        error("Error binding\n");

    return ret;
}

//Accepts client connection request to server 
int acceptClient(int socketfd, sockaddr_in *clientAddr){
    int clientLen = sizeof(clientAddr);
    //accept() returns 0 if success and -1 if fail
    int ret = accept(socketfd,(sockaddr *)clientAddr, &clientLen);

    if(ret < 0)
        error("Error accepting connection\n");

    return ret; 

}

/* CLIENT RELATED FUNCTIONS */

//CURRENTLY NOT WORKING
//Connects to server given the socket file desctiptor, hostname (i.e. "127.0.0.1") and port
void connectToServer(int socketfd, hostent *server, sockaddr_in* serverAddr, char* hostName, int port){

    bzero((char *) serverAddr, sizeof(*serverAddr));
    
    serverAddr->sin_family = AF_INET;

    //Copies bytes from first param to second with given length
    bcopy((char *)&server->h_addr, (char *)&serverAddr->sin_addr.s_addr,server->h_length);
    serverAddr->sin_port = htons(port);

    
    //connect returns 0 if success and -1 if fail
    if(connect(socketfd, (sockaddr *)serverAddr, sizeof(*serverAddr)) < 0)
        error("Error Connecting to server");


}
