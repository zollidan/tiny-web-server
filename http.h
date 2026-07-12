#include <stddef.h>

typedef enum {
    HTTP_OK = 200,
    HTTP_PARTIAL_CONTENT = 206,
    HTTP_BAD_REQUEST = 400,
    HTTP_NOT_FOUND = 404,
    HTTP_METHOD_NOT_ALLOWED = 405,
    HTTP_RANGE_NOT_SATISFIABLE = 416,
    HTTP_INTERNAL_SERVER_ERROR = 500
} HttpStatus;

typedef struct {
    HttpStatus status_code;
    const char *status_text;
    const char *content_type;
    const void *body;
    size_t body_length;
} HttpResponse;

int send_all(int sockfd, const void *data, size_t length);
int send_response(int sockfd, const HttpResponse *response);
void send_not_found(int sockfd);
void send_ok(int sockfd, const char *content_type, size_t body_length);
