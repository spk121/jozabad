#ifndef LOG_H
#define	LOG_H

extern int opt_loglevel;
#define ERR(...)                                  \
    do {                                          \
        if (opt_loglevel >= 1)                    \
            zclock_log("Error: " __VA_ARGS__);    \
    } while(0)
#define WARN(...)                                   \
    do {                                            \
        if (opt_loglevel >= 2)                      \
            zclock_log("Warning: "  __VA_ARGS__);   \
    } while(0)
#define INFO(...)                               \
    do {                                        \
        if (opt_loglevel >= 3)                  \
            zclock_log("Info: "  __VA_ARGS__);  \
    } while(0)
#define NOTE(...)                               \
    do {                                        \
        if (opt_loglevel >= 4)                  \
            zclock_log("Note: "  __VA_ARGS__);  \
    } while(0)
#define TRACE(...)                              \
    do {                                        \
        if (opt_loglevel >= 5)                  \
            zclock_log("Trace: "  __VA_ARGS__); \
    } while(0)

#endif	/* LOG_H */

