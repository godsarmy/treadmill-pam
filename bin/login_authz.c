#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "request.h"

/*
 * Treadmill login_authz independent binary
 */

const char* help_string = "Usage: login_authz [-h] [-4|-6|-u] [-p PORT] [-o OUTPUT_FILE] SERVER USER APP\n";

void errExit(const char* format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, format, args);
    va_end(args);
    perror("");
    exit(1);
}

int main (int argc, char** argv) {
    struct sockaddr_un sockaddrun;
    struct sockaddr_in sockaddrin;
    int srvfd, outfile, ai_family = AF_INET;
    char c, *app, *address, *user;
    unsigned int port = 80;

    while ((c = getopt(argc, argv, "a:p:ho:46u")) >= 0) {
        switch (c) {
            case 'h' :
                errExit(help_string, 0);
            case 'p' :
                port = atoi(optarg);
                break;
            case 'o' :
                outfile = open(optarg,O_WRONLY|O_CREAT, 0644);
                close(1);
                dup2(outfile, 1);
                break;
            case '4' :
                ai_family = AF_INET;
                break;
            case '6' :
                ai_family = AF_INET6;
                break;
            case 'u':
                ai_family = AF_UNIX;
                break;
        }
    }

    if (argc < optind + 3) {
        fprintf(stderr, "Missing necessary parameters\n");
        fprintf(stderr, help_string);
        exit(1);
    }

    memset(&sockaddrin, 0, sizeof(struct sockaddr_in));
    memset(&sockaddrun, 0, sizeof(struct sockaddr_un));

    address = argv[optind];
    user = argv[optind + 1];
    app = argv[optind + 2];

    if (ai_family == AF_INET || ai_family == AF_INET6) {
        struct hostent *host;
        host = gethostbyname(address);

        if(host == NULL) {
            errExit("gethostbyname");
        }
        sockaddrin.sin_family = ai_family;
        sockaddrin.sin_port = htons(port);
        sockaddrin.sin_addr.s_addr = *((unsigned long *)host->h_addr);

        srvfd = socket(ai_family, SOCK_STREAM, 0);
        if ( srvfd < 0 ) {
            errExit("socket()");
        }

        if (connect(srvfd, (struct sockaddr*) &sockaddrin, sizeof(sockaddrin)) == -1) {
            errExit("connect");
        }
    }
    else {
        sockaddrun.sun_family = ai_family;
        strncpy(sockaddrun.sun_path, address, sizeof(sockaddrun.sun_path) - 1);

        srvfd = socket(ai_family, SOCK_STREAM, 0);
        if (srvfd < 0) {
            errExit("socket()");
        }

        if (connect(srvfd, (struct sockaddr *) &sockaddrun, sizeof(sockaddrun)) == -1) {
            errExit("connect");
        }
    }

    int rc = verify(srvfd, address, user, app);
    close(srvfd);

    printf("authz: %s\n", rc ? "false" : "true");
    return rc;
}
