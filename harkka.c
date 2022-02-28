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

void serve(int sockfd) {
    char buff[MAX];
    int len = 1;

    while(len > 0) {
        len = read(sockfd, buff, sizeof(buff));
        write(STDOUT_FILENO, buff, len);
    }
    if (len < 0) ERR("read failed");

}

void newserver(int port) {
    int sockfd, childSocket;
	struct sockaddr_in serverAddr, clientAddr;
    pid_t childpid;
    socklen_t addr_size;

    // create socket
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) ERR("server socket failed");

    // bind and listen
    memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
    if(bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
                ERR("server bind failed");
    if(listen(sockfd, 10) < 0) ERR("server listen failed");

    // create children
    while(1) {
        childSocket = accept(sockfd, (struct sockaddr*)&clientAddr, &addr_size);
        if(childSocket < 0) ERR("child socket failed");
        childpid = fork();
        if(childpid == 0) {
            close(sockfd);
            serve(childSocket);
        } else {
            close(childSocket);
        }
    }
}

#define HELLO "HELLO\n"
#define LOGIN "priihika\n"
void client(int sockfd) {
    char buff[MAX];
    int len = 1, port;

    len = read(sockfd, buff, sizeof(buff));
    write(STDOUT_FILENO, buff, len);
    write(sockfd, HELLO, strlen(HELLO));
    len = read(sockfd, buff, sizeof(buff));
    write(STDOUT_FILENO, buff, len);
    write(sockfd, LOGIN, strlen(LOGIN));
    len = read(sockfd, buff, sizeof(buff));
    write(STDOUT_FILENO, buff, len);
    len = read(sockfd, buff, sizeof(buff));
    write(STDOUT_FILENO, buff, len);
    len = read(sockfd, buff, sizeof(buff));
    write(STDOUT_FILENO, buff, len);
    port = atoi(buff + len - 6);
    printf("Port: %d\n", port);
    newserver(port);
    
    while(len > 0) {
        len = read(sockfd, buff, sizeof(buff));
        write(STDOUT_FILENO, buff, len);
    }
    if (len < 0) ERR("read failed");
}
   
int main()
{
    int sockfd;
    struct addrinfo hints, *serverAddr, *p;
   
    // get server IP, PORT
    memset(&hints, 0, sizeof(struct addrinfo));
    if (getaddrinfo("whx-10.cs.helsinki.fi", "UNIX_TL", &hints, &serverAddr) != 0) ERR("getaddrinfo failed");
   
    // connect the client socket to a server address in the list
	for(p = serverAddr; p != NULL; p = p->ai_next) {
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
    freeaddrinfo(serverAddr);
    close(sockfd);
}