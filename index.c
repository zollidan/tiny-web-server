#include <stdio.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#include "network.h"
#include "utils.h"

#define PORT 7980

int main()
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
    host_addr.sin_port = htons(PORT);
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
