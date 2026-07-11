#include <stdio.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include "utils.h"

#define PORT 7980
#define WEBROOT "./webroot"

int main(int argc, char const *argv[])
{
    int sockfd, new_sockfd, yes = 1;
    struct sockaddr_in host_addr, client_addr;
    socklen_t sin_size;

    printf("Сервер открывается на порту: %d\n", PORT);

    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        fatal("ошибка в создании сокета", __FILE__);
    }

    printf("Сокет успешно создан, дескриптор: %d\n", sockfd);

    if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) == -1) {
        fatal("ошибка в настройке сокета SO_REUSEADDR", __FILE__);
    }

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = PORT;
    host_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(host_addr.sin_zero), '\0', 8);
    
    if ((bind(sockfd, (struct sockaddr *)&host_addr, sizeof(host_addr))) == -1) {
        fatal("ошибка в bind", __FILE__);
    }

    if (listen(sockfd, 20) == -1) {
        fatal("открытие сокета на прослушивание", __FILE__);
    }

    while (1) {
        sin_size = sizeof(struct sockaddr_in);
        new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
        if (new_sockfd == -1)
            fatal("прием соеденинения", __FILE__);

        handle_connection(new_sockfd, &client_addr);
    }
    return 0;
}

void handle_connection(int sockfd, struct sockaddr_in *client_addr_ptr) {
    unsigned char request[500];
    int length; 

    length = recv_line(sockfd, request);

    printf(" Получение запроса от %s:%d \"%s\"\n", inet_ntoa(client_addr_ptr->sin_addr), ntohs(client_addr_ptr->sin_port), request);
}

int get_file_size(int fd) {
    struct stat stat_struct;

    if (fstat(fd, &stat_struct) == -1) 
        return -1;

    return (int) stat_struct.st_size;
}

int recv_line(int sockfd, unsigned char *dest_buffer) {
#define EOL "\r\n"
#define EOL_SIZE 2

    unsigned char *ptr;
    int eol_matched = 0;

    ptr = dest_buffer;
    while (recv(sockfd, ptr, 1, 0) == 1) {
        if (*ptr == EOL[eol_matched]){
            *(ptr+1-EOL_SIZE) = '\0';
            return strlen(dest_buffer);
        } else {
            eol_matched = 0;
        }
        ptr++;
    }
    return 0;
}