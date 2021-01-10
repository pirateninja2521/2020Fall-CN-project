#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <netinet/in.h>

int main(){
    FILE *f = fopen("demo/try.html", "r"); 
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
    f = fopen("demo/try.html", "w");
    fwrite(content, 1, ptr-content, f);
    fwrite(append, 1, strlen(append), f);
    char *UserName = "sam", *message = "AAA";
    char new_tr[10240];
    time_t timer = time(NULL);
    sprintf(new_tr, "                <tr> <td>%s</td> <td>%s</td> <td>%s</td>\n", ctime(&timer), UserName, message);
    fwrite(new_tr, 1, strlen(new_tr), f);
    fwrite(ptr+strlen(append), 1, strlen(ptr)-strlen(append), f);
    fclose(f);
    return 0;
}