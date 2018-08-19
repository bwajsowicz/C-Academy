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
#include <stdbool.h>

#define MAX_EVENTS 10

void Error(char *message)
{
    pError(message);
    exit(1);
}

//TODO: move whole networking to separate class
int CreateSocket()
{
    int newSocket;

    newSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(newSocket < 0)
        Error("Error, can't open new socket!\n");

    return newSocket;
}

void EstablishConnection(int serverSocket, char *argv[])
{
    struct hostent *server;
    struct sockaddr_in serverAddress;
    int port;

    port = atoi(argv[2]);

    server = gethostbyname(argv[1]);
    if (server == NULL)
        Error("Host not found!\n");

    bzero((char *) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;

    bcopy((char *) server->h_addr, (char *) &serverAddress.sin_addr.s_addr, server->h_length);
    serverAddress.sin_port = htons(port);

    if (connect(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) 
        Error("Connection failed!");      
}

void ReadMessage(int socket, char buffer[], int bufferLength)
{
    int result;
    bzero(buffer, bufferLength);

    result = read(socket, buffer, bufferLength);

    if(result < 0)
        Error("Failed to read from the socket!\n");
}

void SendMessage(int socket, char message[], int messageLength)
{
    int result;

    result = send(socket, message, messageLength, 0);
    
    if(result < 0)
        Error("Failed to read from the socket!\n");
}

int GetPlayersId(int socket)
{
    int bufferLength = 10;
    char buffer[bufferLength];

    ReadMessage(socket, buffer, bufferLength);

    printf("%s\n", buffer);

    return std::stoi(buffer);
}

bool IsGameStared(int socket)
{
    int bufferLength = 256;
    char buffer[bufferLength];

    ReadMessage(socket, buffer, bufferLength);

    if(strstr(buffer, "res:rdy"))
        return true;
    else
        return false;
}

void PrintGameStage(int socket)
{
    int bufferLength = 256;
    char buffer[bufferLength];

    ReadMessage(socket, buffer, bufferLength);

    printf("%s starts!\n", buffer);
}

void PrintTable(int socket)
{
    int bufferLength = 2048;
    char buffer[bufferLength];

    ReadMessage(socket, buffer, bufferLength);

    printf("%s", buffer);
}

void PrintHand(int socket)
{
    int bufferLength = 2048;
    char buffer[bufferLength];

    ReadMessage(socket, buffer, bufferLength);

    printf("%s", buffer);
}

int TakeAction(int socket, int id)
{
    int bufferLength = 256;
    int userActionLength = 10;
    char buffer[bufferLength];
    char userAction[userActionLength];

    while(1)
    {
        bzero(userAction, userActionLength);

        ReadMessage(socket, buffer, bufferLength);

        if(strstr(buffer, std::to_string(id).c_str()))
        {
            printf("Your turn! \n");

            ReadMessage(socket, buffer ,bufferLength);
            printf("%s", buffer);

            scanf("%s", userAction);

            SendMessage(socket, userAction, userActionLength);

            if(userAction[0] == '3')
            {
                bzero(buffer, bufferLength);
                ReadMessage(socket, buffer, bufferLength);


                if(strstr(buffer, "call"))
                    continue;
                else
                {
                    printf("How much do you want to call?\n");

                    bzero(userAction, userActionLength);
                    scanf("%s", userAction);

                    SendMessage(socket, userAction, userActionLength);
                }
            }
        }
        else if(strstr(buffer, "res:stageended"))
            break;
        else if(strstr(buffer, "res:roundended"))
            return -1;
        else
        {
            ReadMessage(socket, buffer, bufferLength);
            printf("%s", buffer);
        }
    }
}

void PrintRoundInfo(int socket)
{
    int bufferLength = 256;
    char buffer[bufferLength];

    ReadMessage(socket, buffer, bufferLength);

    printf("%s\n", buffer);
}

int main(int argc, char *argv[])
{
    if(argc < 3)
        Error("Not enough arguments!\n");

    int serverSocket = CreateSocket();
    int id;

    EstablishConnection(serverSocket, argv);

    id = GetPlayersId(serverSocket);

    if(IsGameStared(serverSocket))
    {
        //round loop
        while(1)
        {
            PrintRoundInfo(serverSocket);
            //stage loop (preflop, flop, turn, river)
            while(1)
            {
                PrintGameStage(serverSocket);
                PrintTable(serverSocket);
                PrintHand(serverSocket);
                if(TakeAction(serverSocket, id) == -1) //TODO: change concept for this...
                    break;
            }
        }
    }  

    getchar();
}