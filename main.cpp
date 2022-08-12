#include <algorithm>
#include <iostream>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <vector>

struct sockaddr_in serv_addr;
int sockfd;
char buffer[512] = {0};

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

std::vector<char *> get_response()
{
    printf("odbieram\n");
    char result[512];
    memset(buffer, 0, sizeof(buffer));
    int nDataLength;
    while ((nDataLength = recv(sockfd, buffer, 512, 0)) > 0)
    {
        strcat(result, buffer);
        strcat(result, " ");
    }
    printf("Oto rezultat: %s\n", result);

    std::vector<char *> v;
    if (result[0] == '\0')
        return v;
    bool colon = false;
    char temp[512];
    char xd[1];
    for (int i = 0; i < strlen(result); i++)
    {
        if ((!colon && result[i] == ' ') || (i == strlen(result)))
        {
            printf(temp);
            v.push_back(temp);
            memset(temp, 0, 512);
            continue;
        }
        if (i > 0 && result[i] == ':')
        {
            colon = true;
        }
        xd[0] = result[i];
        strcat(temp, xd);
    }
    return v;
}

bool sendmsg(char *message)
{
    if (send(sockfd, message, strlen(message), 0) == -1)
        return 1;
    else
        return 0;
}

void connectIRC(char *address, int port, char *user, char *nick)
{
    printf("trying to connect\n");
    int valread, client_fd;
    struct sockaddr_in serv_addr;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, address, &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
    }

    if ((client_fd = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
        printf("\nConnection Failed \n");
    }

    char message[512];
    snprintf(message, 512, "NICK %s\n\r", nick);
    sendmsg(message);

    std::vector<char *> parsed = get_response();
    printf("po\n");
    for (auto x : parsed)
    {
        printf("%s ", x);
    }
    memset(message, 0, sizeof(message));

    snprintf(message, sizeof(message), "USER %s * * :%s\n\r", nick, user);
    sendmsg(message);
}

int main(int argc, char const *argv[])
{

    char *address = strdup("127.0.0.1");
    char *user = strdup("Jakub");
    char *nick = strdup("Kumber");
    int port = 8080;
    connectIRC(address, port, user, nick);

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        read(sockfd, buffer, 512);
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