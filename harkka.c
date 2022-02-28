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
    int len = 1;
    while(len > 0) {
        len = read(sockfd, buff, sizeof(buff));
        write(STDOUT_FILENO, buff, len);
        printf("Len: %d\n", len);
    }
}
   
int main()
{
    int sockfd;
    struct addrinfo hints, *server;
   
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) ERR("socket creation failed");
   
    // get server IP, PORT
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    if (getaddrinfo("whx-10.cs.helsinki.fi", "UNIX_TL", &hints, &server) != 0) ERR("getaddrinfo failed");
   
    // connect the client socket to server socket
    if (connect(sockfd, (SA*)server, sizeof(*server)) < 0) ERR("connection with the server failed");
   
    // do the work
    client(sockfd);
   
    // clean up
    freeaddrinfo(server);
    close(sockfd);
}