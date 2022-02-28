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

    len = read(sockfd, buff, sizeof(buff));
    write(STDOUT_FILENO, buff, len);
    write(sockfd, "HELLO", 6);
    while(len > 0) {
        len = read(sockfd, buff, sizeof(buff));
        write(STDOUT_FILENO, buff, len);
        printf("Len: %d\n", len);
    }
    if (len < 0) ERR("read failed");
}
   
int main()
{
    int sockfd;
    struct addrinfo hints, *server, *p;
   
    // get server IP, PORT
    memset(&hints, 0, sizeof(struct addrinfo));
    if (getaddrinfo("whx-10.cs.helsinki.fi", "UNIX_TL", &hints, &server) != 0) ERR("getaddrinfo failed");
   
    // connect the client socket to server address in the list
	for(p = server; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) < 0) {
			perror("looping: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
			perror("looping: connect");
			close(sockfd);
			continue;
		}

		break;
	}

    // do the work
    client(sockfd);
   
    // clean up
    freeaddrinfo(server);
    close(sockfd);
}