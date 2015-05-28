# CS173MP1

A simple File Client and Server for Linux Systems written in C 

#Setting Up

##File Server and Client

If you choose to build on another directory create a Files and ._/RES folder on both Server and Cient directories

*On both Client and Server Directories*
```
mkdir Files
mkdir ._RES
```

#Compiling

##File Server

*Note: Compiled using gcc 4.8.2

```
gcc file_server.c -o [<Server Directory Path>/]file_server -lpthread
```

##File Client

```
gcc file_client.c -o [<Client Directory Path>]file_client
```


##File Client

```
gcc file_client.c 
```

#Running

##File Server

```
./file_server
```
##File Client

```
./file_client <Server IP Address>
```
