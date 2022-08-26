#include <algorithm>
#include <iostream>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <queue>
#include <string>
#include <vector>

const char* NICK_IN_USE = "433";
const char* LOGIN_OK = "001";

std::queue<std::string> to_read;

struct sockaddr_in serv_addr;
int sockfd;
char buffer[512] = {0};

void get_response()
{
    memset(buffer, 0, sizeof(buffer));
    while(!buffer[0])
        read(sockfd, buffer, 512);
    printf("\n[LST]: buffer: %s\n\n", buffer);
    bool colon = false;
    std::string temp = "";
    for(int i = 0; i <= strlen(buffer); i++)
    {
        if(buffer[i] == '\n' || i == strlen(buffer))
        {
            to_read.push(temp);
            temp = "";
        }
        else
        {
            temp = temp + buffer[i];
        }
    }
}

std::vector<std::string> parse_response(std::string resp)
{
    std::vector<std::string> w;
    std::string temp;
    bool colon = false;
    for(int i = 0; i <= resp.size(); i++)
    {
        if(i == resp.size())
        {
            w.push_back(temp);
            break;
        }
        if(i != 0 && resp[i] == ':' && resp[i-1] == ' ')
        {
            colon = true;
            continue;
        }
        if(!colon && resp[i] == ' ')
        {
            w.push_back(temp);
            temp = "";
            continue;
        }
        temp = temp + resp[i];
    }
    return w;
}

bool sendmsg(std::string message)
{
    if (send(sockfd, message.c_str(), message.size(), 0) == -1)
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
int connectIRC(std::string address, int port, std::string user, std::string nick)
{
    std::cout << "[CON]: trying to connect\n";
    int valread, client_fd;
    struct sockaddr_in serv_addr;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cout << "\n[CON]: Socket creation error \n";
        return -1;
    }
    std::cout << "[CON]: sockfd created\n";
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, address.c_str(), &serv_addr.sin_addr) <= 0)
    {
        std::cout << "\n[CON]: Invalid address/ Address not supported \n";
        return -1;
    }
    std::cout << "[CON]: inet_pton works\n";
    if ((client_fd = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
        std::cout << "\n[CON]: Connection Failed \n";
        return -1;
    }
    std::cout << "[CON]: Connected to ip\n";
    
    std::thread listening(get_response);
    listening.detach();

    std::cout << "[CON]: Listening thread launched\n";

    std::string message = "NICK " + nick + "\n\r";
    sendmsg(message);

    message = "USER " + nick + " * * :" + user + "\n\r";
    sendmsg(message);

    std::cout << "[CON]: User data sent\n";
    
    get_response();
    while(!to_read.empty())
    {
        std::vector<std::string> act = parse_response(to_read.front());
        if(act.size() < 2)
        {
            std::cout << "Strange response xd\n";
            for(auto x : act)
            {
                std::cout << x << " ";
            }
            std::cout << '\n';
            break;
        }
        if(act[1] == "433")
        {
            std::cout << "Nickname is already in use\n";
            break;
        }
        else if(act[1][0] == '4')
        {
            std::cout << "EROR: " << act[1] << '\n';
        }
        else if(act[1] == "001")
        {
            std::cout << "Successfully connected and logged to the server\n";
            return 0;
        }
        else if(act[1] == "020")
        {
            sleep(1);
        }
        else
        {
            std::cout << "Unknown code:\n" << act[1] << '\n';
        }
        to_read.pop();
    }
    return 1;
}

int main(int argc, char const *argv[])
{
    // poznan: 150.254.65.52:6667
    // libera: 176.58.122.119:6697
    char *address = strdup("150.254.65.52");
    char *user = strdup("abc abc");
    char *nick = strdup("abcabcabc");
    int port = 6667;

    // char *address = strdup("127.0.0.1");
    // char *user = strdup("Jakub 123");
    // char *nick = strdup("Kumber");
    // int port = 8080;

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