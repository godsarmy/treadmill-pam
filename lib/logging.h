#ifndef LOGGING_H_
#define LOGGING_H_

#include <syslog.h>

#define PAM_MODULE      "pam_treadmill"

/* LOGGING */

#ifdef STDLOG
/* LOG for STDOUT/STDERR */
/*--------------------------------------------------------------------------------------------------------------------*/
#define DBGLOG(...)                                                 \
do {                                                                \
    if(options->debug)                                              \
    {                                                               \
        printf(__VA_ARGS__);                                        \
    }                                                               \
} while (0)

#define INFOLOG(...)                                                \
    printf(__VA_ARGS__)

#define ERRLOG(...)                                                 \
    fprintf(stderr, __VA_ARGS__)

/*--------------------------------------------------------------------------------------------------------------------*/

#else

/* SYSLOG */
/*--------------------------------------------------------------------------------------------------------------------*/
#define SYSLOG(Level, ...)                                          \
do {                                                                \
    openlog(PAM_MODULE, LOG_PID, LOG_AUTH);                         \
    syslog(LOG_ ## Level, ##__VA_ARGS__);                           \
    closelog();                                                     \
} while (0)

#define DBGLOG(...)                                                 \
do {                                                                \
    if(options->debug)                                              \
    {                                                               \
        SYSLOG(DEBUG, __VA_ARGS__);                                 \
    }                                                               \
} while (0)

#define INFOLOG(...)                                                \
    SYSLOG(INFO, __VA_ARGS__)

#define ERRLOG(...)                                                 \
    SYSLOG(ERR, __VA_ARGS__)

/*--------------------------------------------------------------------------------------------------------------------*/
#endif

#endif /* LOGGING_H_ */
