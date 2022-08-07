#include <iostream>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

typedef char Int8;
typedef short int Int16;
typedef int Int32;
typedef unsigned char UInt8;
typedef unsigned short int UInt16;
typedef unsigned int UInt32;

struct sockaddr_in serv_addr;
int sockfd;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

bool sendmsg(char *message)
{
    if (send(sockfd, message, strlen(message), 0) == -1)
        return 1;
    else
        return 0;
}

void connectIRC(char *address, int port, char *user, char *nick1, char *nick2 = strdup(""), char *nick3 = strdup(""))
{
    int valread, client_fd;
    struct sockaddr_in serv_addr;
    char *hello = strdup("Hello from client");
    char buffer[1024] = {0};
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, address, &serv_addr.sin_addr) <= 0)
    {
        printf( "\nInvalid address/ Address not supported \n");
    }

    if ((client_fd = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
        printf("\nConnection Failed \n");
    }

    char *message = strcat(strdup("NICK "), nick1);
    if (sendmsg(message))
        error("ERROR while handling sendto");
}

int main(int argc, char const *argv[])
{

    char *address = strdup("127.0.0.1");
    char *message = strdup("dupa\0\0\0\0");
    char *user = strdup("Jakub");
    char *nick = strdup("Kumber");
    int port = 8080;

    connectIRC(address, port, user, nick);

    char buffer[1024] = {0};
    while (1)
    {
        read(sockfd, buffer, 1024);
        printf("Received:\n%s\n", buffer);
    }
}

/*
    Commands:
        - Message limit is 512 characters
        - Every message ends with CR-LF delimiter (\n\r), so message has space for 510 useful characters
        - Every message longer than 512 characters will be cuted by replacing two last characters with delimiter
        - Message has at least two parts: command and parameters
        - There may be at most 15 parameters
        - The command and the parameters are all separated by a single ASCII space character
        - When parameter is prefixed with colon character
        - When the last parameter is prefixed with a colon character, the value can include spaces
        - Some messages also include a prefix before the command and the command parameters, to indicate message origin

    Server replies:
        - Always includes a prefix
        - Command will be a three-digit code
        - First parameter is always the target of the reply, typically a nick
        - Codes list: https://datatracker.ietf.org/doc/html/rfc2812#section-5


    Connect to server:
        1. Send: `NICK john`
        2. Send: `USER john * * :John Junior
        3. Receive: `:bar.example.com 001 john :Welcome to the Internet Relay Network john!john@foo.example.com
         or
        2. Receive: `:bar.example.com 433 john * :Nickname is already in use


    Private message from john to rory:
        1. (john -> server) PRIVMSG rory :Wahts up Rory?
        2. (server -> rory) :john!john@foo.example.com PRIVMSG rory :Whats up Rory?
*/