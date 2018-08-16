#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>

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

void bind_socket(int socket, int argc, char *argv[])
{

    int port;
    struct sockaddr_in server_address;

    if(argc < 2)
        error("ERROR, no port provided!\n");
    
    setsockopt(socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (int *)1, sizeof(int)); //bind socket forcefully

    port = atoi(argv[1]); 

    bzero((char *) &server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port); //port as network byte order
    server_address.sin_addr.s_addr = INADDR_ANY; 

    if(bind(socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
        error("ERROR, failed to bind socket!");
}

void set_socket_to_listen(int socket)
{
    if(listen(socket, 10) < 0)
        perror("ERROR, can't set socket to listening\n");
}

void set_non_blocking(int socket)
{
    int status = fcntl(socket, F_SETFL, fcntl(socket, F_GETFL, 0) | O_NONBLOCK);

    if(status == -1)
    error("ERROR, calling fcntl.\n");
}

int create_epoll()
{
    int epollfd = epoll_create(MAX_EVENTS);

    if(epollfd == -1)
        perror("ERROR, failed to create epoll!\b");

    return epollfd;
}

void perform_action(int socket)
{
    char buffer[256];
    int n;

    bzero(buffer, 256);
    n = read(socket, buffer, 256);

    printf("%s\n", buffer);
}

int main(int argc, char *argv[])
{
    int server_socket = create_socket();
    int connection_socket;
    int event_count;
    struct sockaddr_in client_address;
    int client_address_length = sizeof(client_address);

    bind_socket(server_socket, argc, argv);
    set_socket_to_listen(server_socket);

    int epollfd = create_epoll();

    struct epoll_event ev, events[MAX_EVENTS];

    ev.events = EPOLLIN;
    ev.data.fd = server_socket;

    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, server_socket, &ev) == -1)
        error("ERROR, failed to add server_socket to the epoll.");

    while(1)
    {
        event_count = epoll_wait(epollfd, events, MAX_EVENTS, -1);

        if(event_count == -1)
            error("ERROR, epoll_wait failed\n");

        for(int i = 0; i < event_count; i++)
        {
            if(events[i].data.fd == server_socket)
            {
                connection_socket = accept(server_socket, (struct sockaddr *) &client_address, (socklen_t *) &client_address_length);

                if(connection_socket == -1)
                    perror("ERROR, failed to create connection socket!\n");

                set_non_blocking(connection_socket);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = connection_socket;

                if(epoll_ctl(epollfd, EPOLL_CTL_ADD, connection_socket, &ev) == -1)
                    error("ERROR, failed to add server_socket to the epoll.");
            }
            else
                perform_action(events[i].data.fd);
        }
    }

    return 0;
}