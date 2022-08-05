#include <iostream>
#include <netdb.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main()
{
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char *address;
    int portno;
    char *message;

    portno = 1234;
    address = "127.0.0.1";
    message = "dupa\0\0\0\0";

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR while opening socket");

    server = gethostbyname(address);
    if (server == NULL)
        error("ERROR, no such host\n");

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR while setting up connecting");

    if (sendto(sockfd, message, sizeof(message), MSG_EOR, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR while handling sendto");
    else
        printf("Sent!\n");

    close(sockfd);
}