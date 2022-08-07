#include <iostream>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

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

struct sockaddr_in serv_addr;
int sockfd;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

bool sendmsg(char *message)
{
    for(int i = 0; i < strlen(message) % 8; i++)
        message = strcat(message, '\0');
    if (sendto(sockfd, message, sizeof(message), MSG_EOR, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        return 1;
    else
        return 0;
}

int connectIRC(char *address, int port, char *user, char *nick1, char *nick2 = "", char *nick3 = "")
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR while opening socket");
    
    struct hostent *server;
    server = gethostbyname(address);
    if (server == NULL)
        error("ERROR, no such host\n");

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR while setting up connecting");


    char *message = strcat("NICK ", nick1);
    if (sendmsg(message))
        error("ERROR while handling sendto");

    return sockfd;
}

int main()
{

    char *address = "127.0.0.1";
    char *message = "dupa\0\0\0\0";
    char *user = "Jakub";
    char *nick = "Kumber";
    int port = 1234;

    close(connectIRC(address, port, user, nick));
}