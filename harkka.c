#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define MAX 1024
#define SA struct sockaddr
#define ERR(msg) { perror(msg); exit(1); }

struct addrinfo *remoteServer;

void sendFile(char *filename, int filelen, int port) {
    int sockfd, filefd, len;
    char *ptr;

    // Prepare the connection
    port = htons(port);
    if(remoteServer->ai_family == AF_INET) ((struct sockaddr_in*)remoteServer)->sin_port = port;
    else ((struct sockaddr_in6*)remoteServer)->sin6_port = port;

    if ((sockfd = socket(remoteServer->ai_family, remoteServer->ai_socktype,
            remoteServer->ai_protocol)) < 0) ERR("sendFile socket failed");
    if (connect(sockfd, remoteServer->ai_addr, remoteServer->ai_addrlen) < 0) {
        ERR("sendFile connect failed");
        close(sockfd);
    }

    // Send the file
    ptr = malloc(filelen);
    if((filefd = open(filename, O_RDONLY)) < 0) ERR("sendFile open failed");
    if (read(filefd, ptr, filelen) < 0) ERR("sendFile read failed");
    while((len = write(sockfd, ptr, filelen)) > 0) ;
    if (len < 0) ERR("sendFile write failed");

    // Clean up
    free(ptr);
    close(sockfd);
}

#define CODE "net-harkka/harkka.c\n"
void serve(int sockfd) {
    char buff[MAX], *ptr;
    struct stat statbuf;
    int len = 1;

    while(len > 0) {
        len = read(sockfd, buff, sizeof(buff));
        write(STDOUT_FILENO, buff, len);
        switch(buff[0]) {
            case 'E':
                    for(int i=0; i<len; i++) {
                        buff[i] = toupper(buff[i]);
                    }
                    write(sockfd, buff, len);
                    break;
            case 'C':
                    write(sockfd, CODE, strlen(CODE));
                    break;
            case 'F':
                    // Reply with the size
                    ptr = strchr(buff + 2, ' ');    // Look for the next space
                    *ptr = '\0';                    // Zero terminate the filename
                    if(stat(buff+2, &statbuf) < 0) ERR("sendFile stat failed");
                    sprintf(buff, "%lld", statbuf.st_size);
                    write(sockfd, buff, strlen(buff));

                    sendFile(buff+2, statbuf.st_size, atoi(buff + len - 6));
                    break;
            case 'A':
                    puts("ACCEPTED!!");
            case 'Q':
                    len = 0;    // exits while()
        }
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
//        } else {
//            close(childSocket);
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
    hints.ai_socktype = SOCK_DGRAM;
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
        remoteServer = p;
		break;
	}

    // do the work
    client(sockfd);
   
    // clean up
    freeaddrinfo(serverAddr);
    close(sockfd);
}