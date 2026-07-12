#pragma once

#include <sys/socket.h>
#include <netinet/in.h>

void handle_connection(int sockfd, struct sockaddr_in *client_addr_ptr);
int get_file_size(int fd);
int recv_line(int sockfd, unsigned char *dest_buffer);