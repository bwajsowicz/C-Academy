#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>

#include <sys/epoll.h>

#define MAX_EVENTS 10

void error(char *message)
{
    perror(message);
    exit(1);
}

int create_socket()
{
    int new_socket;

    new_socket = socket(AF_INET, SOCK_STREAM, 0);

    if(new_socket < 0)
        error("ERROR, can't open new socket!\n");

    return new_socket;
}

void connect_to_server(int server_socket, char *argv[])
{
    struct hostent *server;
    struct sockaddr_in server_address;
    int port;

    port = atoi(argv[2]);

    server = gethostbyname(argv[1]);
    if (server == NULL)
        error("Host not found!\n");

    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;

    bcopy((char *) server->h_addr, (char *) &server_address.sin_addr.s_addr, server->h_length);
    server_address.sin_port = htons(port);

    if (connect(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) 
        error("Connection failed!");      
}

int main(int argc, char *argv[])
{
    if(argc < 3)
        error("Not enough arguments!\n");

    int server_socket = create_socket();

    connect_to_server(server_socket, argv);

    while(1)
    {
        int n;
        char buffer[256] = "XDDDDDDDDDDDDDDDDDDDDDDDDDDD";
        n = send(server_socket, buffer, 256, 0);
        printf("DATA SEND\n");
        getchar();
    }
       
}