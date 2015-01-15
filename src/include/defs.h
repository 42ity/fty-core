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

/*\brief Database error constanst*/
#define BIOS_ERROR_DB DB_ERR

//! Possible error types
enum errtypes {
    DB_ERR = 1,
    BAD_INPUT,
    INTERNAL_ERR,
};

//! Constants for database errors
enum db_err_nos {
    DB_ERROR_INTERNAL = 1,
    DB_ERROR_BADINPUT = 2,
    DB_ERROR_NOTFOUND = 3,
    DB_ERROR_UNKNOWN = 4,
    DB_ERROR_NOTIMPLEMENTED = 5,
    DB_ERROR_DBCORRUPTED = 6,
};

//! Constants for bad input type of error
enum bad_input_err {
    BAD_INPUT_WRONG_INPUT,
    BAD_INPUT_OUT_OF_BOUNDS,
};

enum internal_err {
    NOT_IMPLEMENTED
};

#endif // SRC_INCLUDE_DEFS_H_
