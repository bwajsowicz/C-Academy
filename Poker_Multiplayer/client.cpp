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
#include <string>
#include <string.h>
#include <iostream>

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

void wait_for_response(int socket)
{
    char buffer[256];
    bzero(buffer, 256);

    read(socket, buffer, 256);

    printf("%s\n", buffer);
}

int get_id(int socket)
{
    char buffer[256];
    bzero(buffer, 256);

    read(socket, buffer, 256);

    printf("%s\n", buffer);

    return std::stoi(buffer);
}

void print_gui(int server_socket)
{
    //wait_for_response(server_socket); //plansza
    //wait_for_response(server_socket); //hando

    char buffer[2048];
    bzero(buffer, 2048);    

    read(server_socket, buffer, 2048);
    printf("%s", buffer);

    bzero(buffer, 2048);    

    read(server_socket, buffer, 2048);
    printf("%s", buffer);
}

void take_action(int socket, int id) 
{
    char buffer[256];
    char users_choice[10];
    bzero(buffer, 256);

    read(socket, buffer, 256);

    if(strstr(buffer, std::to_string(id).c_str()))
    {
        printf("Your turn! \n");
        bzero(buffer, 256);
        read(socket, buffer, 256);

        //read users action
        printf("%s \n", buffer);
        scanf("%c", users_choice);

        send(socket, users_choice, 1, 0);

        //wait for servers response
        bzero(buffer, 256);
        read(socket, buffer, 256);
        printf("%s \n", buffer);
    }
    else
    {
        bzero(buffer, 256);
        read(socket, buffer, 256);
        printf("%s \n", buffer);
    }
}

int main(int argc, char *argv[])
{
    if(argc < 3)
        error("Not enough arguments!\n");

    int server_socket = create_socket();
    int id;

    connect_to_server(server_socket, argv);

    id = get_id(server_socket);
    wait_for_response(server_socket); 

    while(1)
    {     
        print_gui(server_socket);
        while(1)
            take_action(server_socket, id);
    }
       
}