/*
 * Treadmill PAM module to unshare network namespace from user ssh session.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sched.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <time.h>

#include <security/pam_appl.h>
/* We are defining a account checking PAM module */
#define PAM_SM_ACCOUNT
#include <security/pam_modules.h>

#if !defined(PAM_EXTERN)
# define PAM_EXTERN extern
#endif

#include "logging.h"
#include "request.h"

#define PAM_MODULE      "pam_treadmill"

#define SOCK_KEY "sock"
#define APP_KEY "app"

struct module_options {
    char *app;          /* File name of app */
    char *sock;         /* sock path for login service */
    int debug:1;        /* Debug enabled */
};

static
void fill_module_option(const char *option, struct module_options *options) {
    char *end, *val;
    if (!option || !*option)
        return;

    if ((end = index(option, '=')) == NULL)
        return;

    val = end + 1; /* option value after '=' */
    if (end <= option || (*val) == '\0') {
        ERRLOG("error: Ignoring invalid option: '%s'", option);
        return;
    }

    while (*val && isspace(*val))
        val++;

    while ((end > option) && (isspace(end[-1])))
        end--;

    if (strncmp(option, SOCK_KEY, end - option) == 0) {
        options->sock = val;
    }

    if (strncmp(option, APP_KEY, end - option) == 0) {
        options->app = val;
    }
}

static
struct module_options module_options_get(int argc, const char **argv) {
    struct module_options opts = {NULL, NULL, 0};
    for (int i = 0; i < argc; i++) {
        fill_module_option(argv[i], &opts);
    }
    return opts;
}

static
int open_unixsocket(const char *unixsocket) {
    struct sockaddr_un sockaddrun;
    memset(&sockaddrun, 0, sizeof(struct sockaddr_un));

    sockaddrun.sun_family = AF_UNIX;
    strncpy(sockaddrun.sun_path, unixsocket, sizeof(sockaddrun.sun_path) - 1);

    int srvfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if ( srvfd < 0 ) {
        return srvfd;
    }

    if (connect(srvfd, (struct sockaddr *) &sockaddrun, sizeof(sockaddrun)) == -1) {
        close(srvfd);
        return 0;
    }
    return srvfd;
}

PAM_EXTERN
int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc,
                        const char **argv) {
    int rc = unshare(CLONE_NEWNET);
    if (rc != 0) {
        ERRLOG("unshare rc: %d", rc);
    }
    return PAM_SUCCESS;
}

PAM_EXTERN
int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc,
                         const char **argv) {
    return PAM_SUCCESS;
}

PAM_EXTERN
int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc,
                     const char **argv) {

    int rc;
    const char *user;
    if ((rc = pam_get_user(pamh, &user, NULL)) != PAM_SUCCESS) {
        ERRLOG("error: Could not retrieve user: %s",
               pam_strerror(pamh, rc));
    }
    struct module_options opts = module_options_get(argc, argv);
    INFOLOG("info: %s, %s, %s", user, opts.sock, opts.app);

    if (opts.sock == NULL || opts.app == NULL) {
        ERRLOG("error: missing sock or app in parameter");
        return PAM_AUTH_ERR;
    }

    /* read app name from app file, the max app name is 128 */
    char app[129];
    FILE *handler = fopen(opts.app, "r");
    if (handler) {
        int read_size = fread(app, sizeof(char), sizeof(app) - 1, handler);
        app[read_size] = '\0';
        fclose(handler);
    }
    else {
        ERRLOG("error: unable to open file: %s", opts.app);
        return PAM_AUTH_ERR;
    }

    int sock = open_unixsocket(opts.sock);
    if (sock <= 0) {
        ERRLOG("error: unable to connect %s", opts.sock);
        return PAM_AUTH_ERR;
    }

    rc = verify(sock, opts.sock, user, app);
    close(sock);
    if (rc == 0) {
        INFOLOG("info: authed %s => %s", user, app);
        return PAM_SUCCESS;
    }
    else {
        ERRLOG("error: not auth %s => %s", user, app);
        return PAM_PERM_DENIED;
    }
}

