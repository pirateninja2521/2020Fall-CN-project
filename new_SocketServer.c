#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

typedef struct{
    int port;
    int listen_fd;
}server; 

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
void send_html(int sockfd, char *type){
    printf("send_html\n");
    FILE *f;
    if (!strcmp(type, "main")){
        f = fopen("mainpage.html", "r"); 
    }
    else if (!strcmp(type, "demopage")){
        f = fopen("demo/demo.html", "r"); 
    }
    fseek(f, 0, SEEK_END);
    int f_size = ftell(f);
    rewind(f);

    char send_message[819200], send_content[819200];
    memset(send_content, 0, 819200);
    fread(send_content, 1, f_size, f);
    fclose(f);

    sprintf(send_message, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n", strlen(send_content));
    send(sockfd, send_message, strlen(send_message), 0);
    send(sockfd, send_content, strlen(send_content), 0);
    printf("%s\n", send_message);
}
void send_media(int sockfd, int start, char *mode){
    printf("send_media\n");
    FILE *f;
    if (!strcmp(mode, "audio")){
        f = fopen("demo/taichi.mp3", "rb"); 
    }else{
        f = fopen("demo/CN_phase1_demo.webm", "rb");  
    }
    fseek(f, 0, SEEK_END);
    int med_size = ftell(f);
    fseek(f, start, SEEK_SET);

    char send_message[8192], send_content[3000000];
    memset(send_content, 0, 8192);
    int part_len = fread(send_content, 1, (med_size-start>=1000000)? 1000000 : med_size-start , f);
    fclose(f);
    char type[64];
    if (!strcmp(mode, "audio")){
        strcpy(type, "Content-Type: audio/mpeg");
    }else{
        strcpy(type, "Content-Type: video/webm");
    }
    sprintf(send_message, "HTTP/1.1 206 Partial Content\r\nContent-Range: bytes %d-%d/%d\r\nContent-Length: %d\r\n%s\r\n\r\n", start, start+part_len-1, med_size, part_len, type);
    send(sockfd, send_message, strlen(send_message), 0);
    send(sockfd, send_content, part_len, 0);
    printf("%s\n", send_message);
}
void handle_message(int sockfd, char UserName[1024], char message[1024]){
    FILE *f = fopen("demo/demo.html", "r"); 
    fseek(f, 0, SEEK_END);
    int f_size = ftell(f);
    rewind(f);

    char content[819200];
    memset(content, 0, 819200);
    fread(content, 1, f_size, f);
    fclose(f);
    const char *append = "<!-- append here -->\n";
    char *ptr = strstr(content, append), copy[819200];
    strcpy(copy, ptr+strlen(append));
    printf("copy %s", copy);
    pthread_mutex_lock(&lock);
    f = fopen("demo/demo.html", "w");
    fwrite(content, 1, ptr-content, f);
    fwrite(append, 1, strlen(append), f);
    char new_tr[10240];
    time_t timer = time(NULL);
    sprintf(new_tr, "                <tr> <td>%s</td> <td>%s</td> <td>%s</td>\n", ctime(&timer), UserName, message);
    fwrite(new_tr, 1, strlen(new_tr), f);
    fwrite(ptr+strlen(append), 1, strlen(ptr)-strlen(append), f);
    fclose(f);
    pthread_mutex_unlock(&lock);
    send_html(sockfd, "demopage");
}
void *handle_connection(void *fd){
    pthread_detach(pthread_self());

    int sockfd = *(int *)fd;
    printf("handle fd: %d\n", sockfd);

    char recv_message[8192];
    memset( recv_message, 0, 8192);
    int recv_len = recv(sockfd, recv_message,8192,0);
    printf("recv message:\n%s",  recv_message);	
    if (strstr(recv_message, "GET / HTTP")!= NULL){
        send_html(sockfd, "main");
    }
    else if (strstr(recv_message, "GET /demo")!= NULL){
        send_html(sockfd, "demopage");
    }
    else if(strstr(recv_message, "GET /CN_phase1_demo.webm")!= NULL){
        char *ptr = strstr(recv_message, "bytes=");
        int start;
        sscanf(ptr, "bytes=%d", &start);
        send_media(sockfd, start, "video");
    }
    else if(strstr(recv_message, "GET /taichi.mp3")!= NULL){
        char *ptr = strstr(recv_message, "bytes=");
        int start;
        sscanf(ptr, "bytes=%d", &start);
        send_media(sockfd, start, "audio");
    }
    else if (strstr(recv_message, "POST")!= NULL){
        printf("!!!!!\n");
        char *ptr1 = strstr(recv_message, "UserName=");
        char *ptr2 = strchr(ptr1, '&');
        *ptr2 = '\0';
        printf("%s %s", ptr1, ptr2+1);
        handle_message(sockfd, ptr1+9, ptr2+1+8);
    }
    else{
        char *send_message="HTTP/1.1 403 Forbidden\r\n";
        send(sockfd, send_message, strlen(send_message), 0);
    }
    
    close(sockfd);
}


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

    pthread_t tid;
    while(1){
        int len = sizeof(cliaddr);
        printf("waiting\n");
        int new_fd = accept(svr.listen_fd, (struct sockaddr*) &cliaddr, (socklen_t*) &len);
        if (new_fd<0){
            fprintf(stderr, "Accept error\n");
        }
        else{
            pthread_create(&tid, NULL, handle_connection, (void *) &new_fd);
        }
    }
    return 0;
}