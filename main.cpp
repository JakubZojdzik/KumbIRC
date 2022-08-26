#include <algorithm>
#include <iostream>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>

const char* NICK_IN_USE = "433";
const char* LOGIN_OK = "001";

struct sockaddr_in serv_addr;
int sockfd;
char buffer[512] = {0};
char response[16][512];

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int parse_response()
{
    memset(buffer, 0, sizeof(buffer));
    read(sockfd, buffer, 512);
    printf("parsing:%s\n", buffer);
    if (buffer[0] == '\0')
        return 0;
    bool colon = false;
    char temp[512] = {0};
    int j = 0, k = 0;

    for (int i = 0; i <= strlen(buffer); i++)
    {
        if(i == strlen(buffer))
        {
            memset(response[k], 0, 512);
            strcpy(response[k], temp);
            printf("dowalam %s\n", temp);
            k++;
            memset(temp, 0, 512);
            break;
        }
        if (buffer[i] == ' ' && !colon)
        {
            memset(response[k], 0, 512);
            strcpy(response[k], temp);
            printf("dowalam %s\n", temp);
            k++;
            memset(temp, 0, 512);
            j = 0;
            continue;
        }
        if (buffer[i] == ':' && i > 0 && !colon)
        {
            if (i > 0)
                colon = true;
            continue;
        }
        temp[j] = buffer[i];
        j++;
    }
    return k;
}

bool sendmsg(char *message)
{
    if (send(sockfd, message, strlen(message), 0) == -1)
        return 1;
    else
        return 0;
}

/*
Output:
0       Connected
-1      Connection error
1       Nick is already in use
2       Unknown server response
3       Missing server response
*/
int connectIRC(char *address, int port, char *user, char *nick)
{
    printf("trying to connect\n");
    int valread, client_fd;
    struct sockaddr_in serv_addr;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, address, &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    if ((client_fd = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    char message[512];
    snprintf(message, 512, "NICK %s\n\r", nick);
    sendmsg(message);
    
    memset(message, 0, sizeof(message));
    snprintf(message, sizeof(message), "USER %s * * :%s\n\r", nick, user);
    sendmsg(message);


    size_t res_size = parse_response();
    for(int i = 0; i < res_size; i++)
    {
        printf("%i: %s\n", i, response[i]);
    }
    
    if(res_size >= 3)
    {
        if(strcmp(response[1], NICK_IN_USE) == 0)
        {
            printf("\nNickname is already in use \n");
            return 1;
        }
        else if(strcmp(response[1], LOGIN_OK) == 0)
        {
            printf("\nConnected \n");
            return 0;
        }
        else
        {
            printf("\nError: %s\n", buffer);
            return 2;
        }
    }
    else
    {
        printf("\nError \n");
        return 3;
    }
}

int main(int argc, char const *argv[])
{
    char *address = strdup("127.0.0.1");
    char *user = strdup("Jakub 123");
    char *nick = strdup("Kumber");
    int port = 8080;

    connectIRC(address, port, user, nick);
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
        2. Send: `USER john * * :John Junior`
        3. Receive: `:bar.example.com 001 john :Welcome to the Internet Relay Network john!john@foo.example.com`
         or
        2. Receive: `:bar.example.com 433 john * :Nickname is already in use`


    Private message from john to rory:
        1. (john -> server) PRIVMSG rory :Wahts up Rory?
        2. (server -> rory) :john!john@foo.example.com PRIVMSG rory :Whats up Rory?
*/