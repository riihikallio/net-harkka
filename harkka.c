#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#define MAX 1024
#define SA struct sockaddr
#define ERR(msg) { perror(msg); exit(1); }


void client(int sockfd)
{
    char buff[MAX];
    int len;
    while((len = read(sockfd, buff, sizeof(buff))) > 0) {
        write(STDOUT_FILENO, buff, len);
    }
}
   
int main()
{
    int sockfd;
    struct addrinfo hints, *server;
   
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) ERR("socket creation failed");
   
    // get server IP, PORT
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    if (getaddrinfo("whx-10.cs.helsinki.fi", "UNIX_TL", &hints, &server) != 0) ERR("getaddrinfo failed");
   
    // connect the client socket to server socket
    if (connect(sockfd, (SA*)server, sizeof(*server)) != 0) ERR("connection with the server failed");
   
    // do the work
    client(sockfd);
   
    // clean up
    freeaddrinfo(server);
    close(sockfd);
}