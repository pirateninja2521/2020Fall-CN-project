#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
using namespace std; 

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
    FILE *f_video = fopen("CN_phase1_demo.webm", "rb"); 
    if (f_video == NULL){
        fprintf(stderr, "open file error\n");
    }fseek(f_video, 0, SEEK_END);
    int vid_size = ftell(f_video);
    rewind(f_video);
    while(1){
        int len = sizeof(cliaddr);
        printf("waiting\n");
        int new_fd = accept(svr.listen_fd, (struct sockaddr*) &cliaddr, (socklen_t*) &len);
        if (new_fd<0){
            fprintf(stderr, "Accept error\n");
        }

        char recv_buf[8192];
        memset(recv_buf, 0, 8192);
        int recv_len = recv(new_fd,recv_buf,8192,0);
        printf("%s", recv_buf);
        // char *end = strchr(recv_buf, '\n');
        // if (end) *end = '\0';
        // printf("%s\n", recv_buf);
        const char *get_html="GET / HTTP", *get_video = "GET /CN_phase1_demo.webm";
        char content[1000005], buf[1100000];
        FILE *f;
        if (!strncmp(recv_buf, get_html, strlen(get_html))){
            printf("get html!\n");
            f = fopen("mainpage.html", "r"); 
            if (f == NULL){
                fprintf(stderr, "open file error\n");
            }
            fseek(f, 0, SEEK_END);
            int f_size = ftell(f);
            rewind(f);
            memset(content, 0, 2*f_size);
            fread(content, 1, f_size, f);
            fclose(f);
            // printf("%ld\n", strlen(content));
            sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n%s", strlen(content), content);
            send(new_fd, buf, strlen(buf), 0);
        }
        else if (!strncmp(recv_buf, get_video, strlen(get_video))){
            char *ptr = strstr(recv_buf, "bytes=");
            int rs;
            sscanf(ptr, "bytes=%d", &rs);
            printf("rs %d\n", rs);
            printf("get video!\n");
            memset(content, 0, 1000005);
            int part_len;
            fseek(f_video, rs, SEEK_SET);
            part_len = fread(content, 1, (vid_size-rs>=1000000) ? 1000000:(vid_size-rs) , f_video);
            printf("part len %d\n", part_len);
            sprintf(buf, "HTTP/1.1 206 Partial Content\r\nContent-Range: bytes %d-%d/%d\r\nContent-Length: %d\r\nContent-Type: video/webm\r\n\r\n", rs, rs+part_len-1, vid_size-1, part_len);
            printf("buf size: %ld\n%s", strlen(buf), buf);
            send(new_fd, buf, strlen(buf), 0);
            send(new_fd, content, part_len, 0);
            // printf("%ld\n", strlen(content));
            
        }
        else{
            printf("get html!\n");
            f = fopen("mainpage.html", "r"); 
            if (f == NULL){
                fprintf(stderr, "open file error\n");
            }
            fseek(f, 0, SEEK_END);
            int f_size = ftell(f);
            rewind(f);
            memset(content, 0, 2*f_size);
            fread(content, 1, f_size, f);
            fclose(f);
            // printf("%ld\n", strlen(content));
            sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n%s", strlen(content), content);
            send(new_fd, buf, strlen(buf), 0);
        }
        close(new_fd);
        // sleep(1);
    }
    return 0;
}