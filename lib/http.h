#ifndef HTTP_H
#define HTTP_H

struct http_response {
    /* total buffer */
    char *buffer;
    size_t size;

    /* http knowledge */
    int status;
    char *message;
    char *body;
};

struct http_response request(const int socket_fd, const char *address,
                             const char *path, const char *payload);


/* http reponse buffer related method */
int alloc_buffer(struct http_response *response,
                 const size_t size);
int realloc_buffer(struct http_response *response);
void free_buffer(struct http_response *response);

#endif
