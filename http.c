#include <sys/socket.h>
#include <stdio.h>

#include "http.h"

int send_all(int sockfd, const void *data, size_t length)
{
    const unsigned char *buffer = data;
    size_t total_sent = 0;

    while (total_sent < length) 
    {
        ssize_t sent = send(
            sockfd,
            buffer + total_sent,
            length - total_sent,
            0
        );
        if (sent <= 0) return 0;

        total_sent += (size_t)sent;
    }
    
    return 1;
}

int send_response(int sockfd, const HttpResponse *response)
{
    char headers[512];
    const char *content_type = response->content_type;

    if (content_type == NULL) {
        content_type = "application/octet-stream";
    }

    int headers_length = snprintf(
        headers,
        sizeof(headers),
        "HTTP/1.0 %d %s\r\n"
        "Server: Tiny Web Server\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "\r\n",
        response->status_code,
        response->status_text,
        content_type,
        response->body_length
    );

    if (headers_length < 0 ||
        (size_t)headers_length >= sizeof(headers)) {
        return 0;
    }

    if (!send_all(sockfd, headers, (size_t)headers_length)) {
        return 0;
    }

    if (response->body != NULL && response->body_length > 0) {
        return send_all(
            sockfd,
            response->body,
            response->body_length
        );
    }

    return 1;
}

void send_not_found(int sockfd)
{
    static const char body[] =
        "<html>"
        "<head><title>404 Not Found</title></head>"
        "<body><h1>URL not found</h1></body>"
        "</html>";

    HttpResponse response = {
        .status_code = HTTP_NOT_FOUND,
        .status_text = "Not Found",
        .content_type = "text/html; charset=utf-8",
        .body = body,
        .body_length = sizeof(body) - 1
    };

    send_response(sockfd, &response);
}

void send_ok(int sockfd, const char *content_type, size_t body_length) {

    HttpResponse response = {
        .status_code = HTTP_OK,
        .status_text = "OK",
        .content_type = content_type,
        .body = NULL,
        .body_length = body_length
    };

    send_response(sockfd, &response);
}
