#ifndef SC_CONFIG_H
#define SC_CONFIG_H

#define SCRCPY_VERSION "4.0"

#ifndef PREFIX
#define PREFIX "/usr/local"
#endif

// PORTABLE not defined - use ADB environment variable

#define DEFAULT_LOCAL_PORT_RANGE_FIRST 27183
#define DEFAULT_LOCAL_PORT_RANGE_LAST 27199

#define SERVER_DEBUGGER 0

#define HAVE_V4L2 0

#define HAVE_USB 1

/* macOS does not have SOCK_CLOEXEC or accept4 */

#define HAVE_STRDUP 1
#define HAVE_ASPRINTF 1
#define HAVE_VASPRINTF 1
#define HAVE_NRAND48 1
#define HAVE_JRAND48 1
/* HAVE_REALLOCARRAY not defined - provided by compat.c */

#endif
