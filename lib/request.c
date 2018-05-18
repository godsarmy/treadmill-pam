
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "logging.h"
#include "http.h"
#include "request.h"

#define STATUS_OK 200
#define BUFFER_SIZE 160

const char AUTH_MAGIC[] = "\"auth\": ";
const char TRUE_MAGIC[] = "true";

static
int check_auth(char *body) {
    /* check if auth from response boday */

    if (body == NULL) {
        return 1;
    }

    /* check if body has auth key in json */
    char *pch = strstr(body, AUTH_MAGIC);
    if (pch == NULL) {
        return 1;
    }

    int len_auth = strlen(AUTH_MAGIC);
    int len_true = strlen(TRUE_MAGIC);
    /* check value of auth, ensure body length longer than magic string */
    if ( body + strlen(body) < pch + len_auth + len_true ||
            strncmp(pch + len_auth, TRUE_MAGIC, len_true) != 0) {
        return 1;
    }

    return 0;
}

/* verify login to send user, app to login service, return 0 if auth passed */
int verify(const int socket_fd, const char *address, const char *user,
           const char *app) {

    /* app name less than 128 chars
     * proid should be less than 32 chars
     * user id + mail should be less than 128 chars for personal container */
    char body[BUFFER_SIZE];
    char path[BUFFER_SIZE];

    snprintf(path, sizeof(path), "%s/login/container", user);
    snprintf(body, sizeof(body), "{\"pk\": \"%s\"}", app);

    /* send http request and got response */
    struct http_response response = request(socket_fd, address, path, body);
    INFOLOG("HTTP status: %d, body: %s\n", response.status, response.body);

    int rc = 1;
    /* if stauts is not OK, we regard not authorized */
    if (response.status == STATUS_OK) {
        rc = check_auth(response.body);
    }

    free_buffer(&response);
    return rc;
}
