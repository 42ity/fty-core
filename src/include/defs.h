#ifndef SRC_INCLUDE_DEFS_H_
#define SRC_INCLUDE_DEFS_H_

// marker to tell humans and GCC that the unused parameter is there for some
// reason (i.e. API compatibility) and compiler should not warn if not used
#if !defined(UNUSED_PARAM) && defined (__GNUC__)
# define UNUSED_PARAM __attribute__ ((__unused__))
#else
# define UNUSED_PARAM
#endif

/* Implemented in defs.c, link with that if you use any of these values */
extern const char* DB_SOCK;
extern const char* FILIP_SOCK;
extern const char* CLI_SOCK;

extern const char* DRIVER_NMAP_SOCK;
extern const char* DRIVER_NMAP_REPLY;

//TODO: fix that better - this will work until we'll don't touch the initdb.sql
#define UI_PROPERTIES_CLIENT_ID 5
#define DUMMY_DEVICE_ID 1

/*\brief Database error constanst*/
// TODO change to enum
#define DB_ERROR_INTERNAL 1
#define DB_ERROR_BADINPUT 2
#define DB_ERROR_NOTFOUND 3
#define DB_ERROR_UNKNOWN 4
#define DB_ERROR_NOTIMPLEMENTED 5
#define DB_ERROR_DBCORRUPTED 6
#define DB_ERROR_NOTHINGINSERTED 7

#define BIOS_ERROR_DB 1

#endif // SRC_INCLUDE_DEFS_H_
