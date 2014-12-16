#ifndef SRC_INCLUDE_DEFS_H_
#define SRC_INCLUDE_DEFS_H_

// Trick to avoid conflict with CXXTOOLS logger, currently the BIOS code
// prefers OUR logger macros
#if defined(LOG_CXXTOOLS_H) || defined(CXXTOOLS_LOG_CXXTOOLS_H)
# undef log_error
# undef log_debug
# undef log_info
# undef log_fatal
# undef log_warn
#else
# define LOG_CXXTOOLS_H
# define CXXTOOLS_LOG_CXXTOOLS_H
#endif

// marker to tell humans and GCC that the unused parameter is there for some
// reason (i.e. API compatibility) and compiler should not warn if not used
#if !defined(UNUSED_PARAM) && defined (__GNUC__)
# define UNUSED_PARAM __attribute__ ((__unused__))
#else
# define UNUSED_PARAM
#endif

static const char* DB_SOCK = "ipc://@/poord/persistence";
static const char* FILIP_SOCK = "ipc://@/poord/netlogic";
static const char* CLI_SOCK = "ipc://@/poord/cli";

static const char* DRIVER_NMAP_SOCK = "ipc://@/bios/driver/nmap";
static const char* DRIVER_NMAP_REPLY = "ipc://@/bios/driver/nmap_reply";

//TODO: fix that better - this will work until we'll don't touch the initdb.sql
#define UI_PROPERTIES_CLIENT_ID 5

/*\brief Database error constanst*/
#define DB_ERROR_INTERNAL 1
#define DB_ERROR_BADINPUT 2
#define DB_ERROR_NOTFOUND 3
#define DB_ERROR_UNKNOWN 4
#define DB_ERROR_NOTIMPLEMENTED 5

#define BIOS_ERROR_DB 1

#endif // SRC_INCLUDE_DEFS_H_

