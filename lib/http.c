#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "http.h"

#define DEFAULT_RESP_SIZE 1024

const char BODY_DELIMIER[] = "\r\n\r\n";


/* buffer utiliities */
/* alloc_buffer, return size of buffer allocated */
int alloc_buffer(struct http_response *response,
                 const size_t size) {
    /* in case it is allocated before */
    free_buffer(response);

    if (size <= 0) {
        return size;
    }

    response->buffer = (char *) calloc(size, 1);
    if (response->buffer != NULL) {
        response->size = size;
    }
    else {
        response->size = 0;
    }
    return response->size;
}

/* re-allocate to increase the buffer size, return size of buffer allocated */
int realloc_buffer(struct http_response *response) {
    size_t new_size = response->size * 2;
    response->buffer = (char *) realloc(response->buffer, new_size);

    if (response->buffer != NULL) {
        response->size = new_size;
    }
    else {
        response->size = 0;
    }
    return response->size;
}

/* free the response buffer */
void free_buffer(struct http_response *response) {
    if (response != NULL && response->size > 0) {
        free(response->buffer);
        response->buffer = NULL;
        response->size = 0;
    }
}

/* Get http response object from string buffer */
void parse_response(struct http_response *response) {

    if (response->size == 0)
        return;

    char *buffer = response->buffer;

    int length = strlen(buffer);
    char *head_end = strstr(buffer, BODY_DELIMIER);

    if (head_end == NULL ||
            head_end + strlen(BODY_DELIMIER) > buffer + length) {
        response->body = NULL;
    }
    else {
        response->body = head_end + strlen(BODY_DELIMIER);
    }

    /* First line of response should status message line */
    char *pch = strtok(buffer, "\n");
    if (pch == NULL) {
        response->message = NULL;
        response->status = 0;
    }
    else {
        response->message = pch;
        char protocol[32];
        int status;
        int rc = sscanf(pch, "%s %d", protocol, &status);
        if (rc == 2) {
            response->status = status;
        }
        else {
            response->status = 0;
        }
    }
}

static
int send_req(const int socket_fd, const char *buf, const size_t buf_len) {
    /* send full http request via socket */
    unsigned int bytes_sent = 0;
    unsigned int bytes_remaining = buf_len;
    while (bytes_remaining > 0) {
        int count = write(socket_fd, buf + bytes_sent, bytes_remaining);
        if (count < 0) {
            return -1;
        }
        bytes_sent += count;
        bytes_remaining -= count;
    }
    return bytes_sent;
}

static
int recv_resp(const int socket_fd, struct http_response * resp) {
    /* receive http response and put into struct buffer */
    int rwn = 1;
    unsigned int bytes_read = 0;
    while ( rwn > 0 ) {
        rwn = read(socket_fd, resp->buffer + bytes_read, resp->size - bytes_read);
        if (rwn == -1 && errno == EINTR)
            continue;
        if (rwn < 0) {
            return rwn;
        }

        bytes_read += rwn;
        if (bytes_read >= resp->size - 1) {
            if (realloc_buffer(resp) == 0) {
                /* this means realloc failed */
                return -1;
            }
        }
    }
    return bytes_read;
}

struct http_response request(const int socket_fd, const char *address,
                             const char *path, const char *payload) {

    struct http_response response = {NULL, 0, 0, NULL, NULL};

    int request_len = 100 + strlen(path) + strlen(address) + strlen(payload);
    char *request = calloc(request_len, 1);

    if (request == NULL) {
        return response;
    }

    /* create http request */
    snprintf(
        request,
        request_len,
        "POST %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-agent: simple-http client\r\n"
        "Content-Length: %zu\r\n"
        "\r\n%s",  /* body */
        path, address, strlen(payload), payload
    );

    /* send http request */
    int sent = send_req(socket_fd, request, strlen(request));
    free(request);

    /* failed to send request */
    if (sent < 0) {
        return response;
    }
    shutdown(socket_fd, SHUT_WR);

    int size = alloc_buffer(&response, DEFAULT_RESP_SIZE);
    if (size == 0) {
        return response;
    }
    int recv = recv_resp(socket_fd, &response);
    if (recv < 0) {
        return response;
    }

    parse_response(&response);
    return response;
}
