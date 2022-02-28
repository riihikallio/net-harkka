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
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAX 1024
#define SA struct sockaddr
#define ERR(msg) { perror(msg); exit(1); }

struct addrinfo *remoteServer;

// TODO: Handle SIGCHLD, SIGINT

// Read full lines ending in \n
int myRead(int sockfd, char *buff, int size) {
    int cnt = 0, n;

    do {
        n = read(sockfd, buff + cnt, size - cnt);
        if (n == 0) break;
        if (n < 0) return(n);
        cnt += n;
    } while (buff[cnt-1] != '\n' && cnt < size);
    return(cnt);
}

void sendFile(char *filename, int filelen, int port) {
    struct addrinfo remote;
    int sockfd, filefd, len;
    char *ptr;
    memcpy(&remote, remoteServer, sizeof(struct addrinfo));

    // Prepare the connection
    ((struct sockaddr_in*)remote.ai_addr)->sin_port = htons(port);
    if ((sockfd = socket(remote.ai_family, remote.ai_socktype,
            remote.ai_protocol)) < 0) ERR("sendFile socket failed");
    if (connect(sockfd, remote.ai_addr, remote.ai_addrlen) < 0) {
        ERR("sendFile connect failed");
        close(sockfd);
    }

    // Send the file
    ptr = malloc(filelen);
    if ((filefd = open(filename, O_RDONLY)) < 0) ERR("sendFile open failed");
    if ((len = read(filefd, ptr, filelen)) < 0) ERR("sendFile read failed");
    if ((len = write(sockfd, ptr, filelen)) < 0) ERR("sendFile write failed");

    // Clean up
    free(ptr);
    close(sockfd);
}

// Serve a single connection
void serve(int sockfd) {
    char buff[MAX], szstr[20], *ptr;
    struct stat statbuf;
    int len = 1;

    while(len > 0) {
        len = myRead(sockfd, buff, sizeof(buff));
        write(STDOUT_FILENO, buff, len);
        switch(buff[0]) {
            case 'E':
                    for(int i=0; i<len; i++) {
                        buff[i] = toupper(buff[i]);
                    }
                    write(sockfd, buff, len);
                    break;
            case 'C':
                    #define CODE "net-harkka/harkka.c\n"
                    write(sockfd, CODE, strlen(CODE));
                    break;
            case 'F':
                    // Reply with the size
                    ptr = strchr(buff + 2, ' ');    // Look for the next space
                    *ptr = '\0';                    // Zero terminate the filename
                    if (stat(buff+2, &statbuf) < 0) ERR("sendFile stat failed");
                    sprintf(szstr, "%ld\n", statbuf.st_size);
                    write(sockfd, szstr, strlen(szstr));
                    // Send the file on another connection
                    sendFile(buff+2, statbuf.st_size, atoi(ptr + 1)); // Last is the portnumber
                    break;
            case 'A':
            case 'Q':
                    len = 0;    // exits while()
        }
    }
    if (len < 0) ERR("read failed");
}

// Fork a child for each connection
void newserver(int port) {
    int serverfd, childfd;
	struct sockaddr_in serverAddr, clientAddr;
    pid_t childpid;
    socklen_t addr_size;

    // Create socket
	if((serverfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) ERR("server socket failed");

    // Bind and listen
    memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(serverfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
                ERR("server bind failed");
    if (listen(serverfd, 10) < 0) ERR("server listen failed");

    // Fork children
    while (1) {
        addr_size = sizeof(clientAddr);
        childfd = accept(serverfd, (struct sockaddr*)&clientAddr, &addr_size);
        if (childfd < 0) ERR("server accept failed");
        childpid = fork();
        if (childpid == 0) {
            close(serverfd);
            serve(childfd);
            close(childfd);
            exit(0);    // child process
        } else {
            close(childfd);
        }
    }
}

void client(int sockfd) {
    char buff[MAX];
    int len = 1, port;

    len = myRead(sockfd, buff, sizeof(buff));
    write(STDOUT_FILENO, buff, len);
    #define HELLO "HELLO\n"
    write(sockfd, HELLO, strlen(HELLO));
    len = myRead(sockfd, buff, sizeof(buff));
    write(STDOUT_FILENO, buff, len);
    #define LOGIN "priihika\n"
    write(sockfd, LOGIN, strlen(LOGIN));
    len = myRead(sockfd, buff, sizeof(buff));
    write(STDOUT_FILENO, buff, len);
    len = myRead(sockfd, buff, sizeof(buff));
    write(STDOUT_FILENO, buff, len);
    port = atoi(buff + len - 6);
    newserver(port);
    
    while (len > 0) {
        len = myRead(sockfd, buff, sizeof(buff));
        write(STDOUT_FILENO, buff, len);
    }
    if (len < 0) ERR("read failed");
}

int main()
{
    int sockfd;
    struct addrinfo hints, *serverAddr, *p;
    char s[INET6_ADDRSTRLEN];
   
    // get server IP & PORT
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo("whx-10.cs.helsinki.fi", "UNIX_TL", &hints, &serverAddr) != 0) ERR("getaddrinfo failed");
   
    // connect the client socket to a server address in the list
	for (p = serverAddr; p != NULL; p = p->ai_next) {
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
    if (p == NULL) ERR("client connection failed");
    remoteServer = p;

    // do the work
    client(sockfd);
   
    // clean up (not really reached)
    freeaddrinfo(serverAddr);
    close(sockfd);
}