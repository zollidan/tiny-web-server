#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "network.h"
#include "utils.h"
#include "counter.h"
#include "http.h"

#define WEBROOT "./webroot"

void handle_connection(int sockfd, struct sockaddr_in *client_addr_ptr) {
    unsigned char *ptr, request[500], resource[500];
    int length; 
    int fd;

    length = recv_line(sockfd, request);
    if (length <= 0) return;

    printf(" Получение запроса от %s:%d \"%s\"\n", 
           inet_ntoa(client_addr_ptr->sin_addr), 
           ntohs(client_addr_ptr->sin_port), 
           request);

    ptr = (unsigned char *)strstr((char *)request, "HTTP/");
    if (ptr == NULL) {
        printf("это не http запрос!\n");
        shutdown(sockfd, SHUT_RDWR);
        return;
    }

    *ptr = 0;
    ptr = NULL;

    if (strncmp((char *)request, "GET ", 4) == 0) {
        ptr = request + 4;
    } else if (strncmp((char *)request, "HEAD ", 5) == 0) {
        ptr = request + 5;
    }
    
    if (ptr == NULL) {
        printf("\tэто неизвестный запрос!\n");
        shutdown(sockfd, SHUT_RDWR);
        return;
    }

    while (*ptr == ' ') ptr++;
    int path_len = strlen((char *)ptr);
    if (path_len > 0 && ptr[path_len - 1] == ' ') {
        ptr[path_len - 1] = 0;
    }

    path_len = strlen((char *)ptr);
    if (path_len > 0 && ptr[path_len - 1] == '/') {
        strcat((char *)ptr, "index.html"); 
    }

    strcpy((char *)resource, WEBROOT);
    strcat((char *)resource, (char *)ptr);

    fd = open((char *)resource, O_RDONLY);
    printf("\tОткрытие: '%s'\n", resource);

    if (fd == -1) {
        send_not_found(sockfd); 
    } else {
        if ((length = get_file_size(fd)) == -1) {
            fatal("при получении размера файла ресурса", __FILE__);
        }

        if (strncmp((char *)request, "GET", 3) == 0) {
            unsigned char *file_buffer = (unsigned char *) malloc(length);
            if (file_buffer == NULL) {
                fatal("при выделении памяти под чтение ресурса", __FILE__);
            }
                
            read(fd, file_buffer, length);

            unsigned char *tag = memmem(file_buffer, length, COUNTER_TAG, strlen(COUNTER_TAG));
            if (tag != NULL) {
                char number[16];
                size_t counter_tag_length = strlen(COUNTER_TAG);
                snprintf(number, sizeof(number), "%06d", next_visitor_number());
                size_t number_length = strlen(number);
                
                send_ok(sockfd, "text/html; charset=utf-8", length - counter_tag_length + number_length);
                send_all(sockfd, file_buffer, tag - file_buffer);
                send_all(sockfd, number, number_length);
                unsigned char *rest = tag + counter_tag_length;
                send_all(sockfd, rest, length - (rest - file_buffer));
            } else {
                send_ok(sockfd, "text/html; charset=utf-8", length);
                send_all(sockfd, file_buffer, length);
            }
            free(file_buffer);
        } else {
            send_ok(sockfd, "text/html; charset=utf-8", length);
        }
        close(fd);
    }

    shutdown(sockfd, SHUT_RDWR);
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
        if (*ptr == EOL[eol_matched]) {
            eol_matched++;
            if (eol_matched == EOL_SIZE) {
                *(ptr + 1 - EOL_SIZE) = '\0';
                return strlen((char *)dest_buffer);
            }
        } else {
            eol_matched = 0;
        }
        ptr++;
    }
    return 0;
}
