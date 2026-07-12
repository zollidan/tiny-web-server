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

#define WEBROOT "./webroot"
#define COUNTER_FILE "counter.txt"
#define COUNTER_TAG  "<!--#COUNTER-->"

int next_visitor_number() {
    int count = 0;

    FILE *fp = fopen(COUNTER_FILE, "r");
    if (fp != NULL) {
        fscanf(fp, "%d", &count);
        fclose(fp);
    }
    count++;
    fp = fopen(COUNTER_FILE, "w");
    if (fp != NULL) {
        fprintf(fp, "%d", count);
        fclose(fp);
    }
    return count;
}

int send_string(int sockfd, unsigned char *buffer) {
    int sent_bytes, bytes_to_sent;
    bytes_to_sent = strlen(buffer);
    while (bytes_to_sent > 0)
    {
        sent_bytes = send(sockfd, buffer, bytes_to_sent, 0);
        if (sent_bytes == -1) return 0;

        bytes_to_sent -= sent_bytes;
        buffer += sent_bytes;
    }
    return 1;
}

void send_not_found(int sockfd) {
    printf(" 404 Not Found\n");
    send_string(sockfd, "HTTP/1.0 404 NOT FOUND\r\n");
    send_string(sockfd, "Server: Tiny Web Server\r\n\r\n");
    send_string(sockfd, "<html><head><title>404 Not Found</title></head>");
    send_string(sockfd, "<body><hl>URL not found</hl></body></html>\r\n");
}

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
        printf("\t200 OK\n");
        send_string(sockfd, "HTTP/1.0 200 OK\r\n");
        send_string(sockfd, "Server: Tiny Web Server\r\n\r\n");

        if (strncmp((char *)request, "GET", 3) == 0) {
            if ((length = get_file_size(fd)) == -1) {
                fatal("при получении размера файла ресурса", __FILE__);
            }
            
            unsigned char *file_buffer = (unsigned char *) malloc(length);
            if (file_buffer == NULL) {
                fatal("при выделении памяти под чтение ресурса", __FILE__);
            }
                
            read(fd, file_buffer, length);

            unsigned char *tag = memmem(file_buffer, length, COUNTER_TAG, strlen(COUNTER_TAG));
            if (tag != NULL) {
                char number[16];
                snprintf(number, sizeof(number), "%06d", next_visitor_number());
                
                send(sockfd, file_buffer, tag - file_buffer, 0);
                send(sockfd, number, strlen(number), 0);
                unsigned char *rest = tag + strlen(COUNTER_TAG);
                send(sockfd, rest, length - (rest - file_buffer), 0);
            } else {
                send(sockfd, file_buffer, length, 0);
            }
            free(file_buffer);
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