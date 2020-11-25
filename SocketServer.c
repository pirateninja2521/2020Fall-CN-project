#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

typedef struct{
    int port;
    int listen_fd;
}server; 

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s [port]\n", argv[0]);
        exit(1);
    }
    server svr;
    svr.port=atoi(argv[1]);
    svr.listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in seraddr;
    struct sockaddr_in cliaddr;
    memset(&seraddr, 0, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_port = htons(svr.port);
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (bind(svr.listen_fd, (struct sockaddr*)&seraddr, sizeof(seraddr)) < 0) {
        fprintf(stderr, "Bind error\n");
    }
    if (listen(svr.listen_fd, 1024)<0){
        fprintf(stderr, "Listen error\n");
    }
    while(1){
        int len = sizeof(cliaddr);
        int new_fd = accept(svr.listen_fd, (struct sockaddr*) &cliaddr, (socklen_t*) &len);
        if (new_fd<0){
            fprintf(stderr, "Accept error\n");
        }
        char buf[128] = "Hello!\n";
        write(new_fd, buf, strlen(buf));
        close(new_fd);
    }
    return 0;
}