#include <stdio.h>
#include <assert.h>
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

        char recv_buf[8192];
        memset(recv_buf, 0, 8192);
        int recv_len = recv(new_fd,recv_buf,8192,0);
        // printf("%s", recv_buf);

        FILE *f = fopen("mainpage.html", "r"); 
        if (f == NULL){
            fprintf(stderr, "open file error\n");
        }
        fseek(f, 0, SEEK_END);
        int f_size = ftell(f);
        rewind(f);
        char content[4096];
        memset(content, 0, 4096);
        fread(content, 1, f_size, f);
        fclose(f);
        // printf("%ld\n", strlen(content));
        char buf[8192];
        sprintf(buf, "HTTP/2 200 OK\r\nContent-Length: %ld\r\n\r\n%s", strlen(content), content);
        send(new_fd, buf, strlen(buf), 0);
        close(new_fd);
    }
    return 0;
}