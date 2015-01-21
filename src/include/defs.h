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

/**
 * \brief Backward compatibility macro
 *
 * BIOS_ERROR_DB is long and contains BIOS which is temporal name and should be
 * replaced with something better, but to get stuff working in the meantime,
 * let's use compatibility macro.
 *
 */
#define BIOS_ERROR_DB DB_ERR

//! Possible error types
enum errtypes {
    //! First error should be UNKNOWN as it maps to zero and zero is weird
    UNKNOWN_ERR,
    DB_ERR,
    BAD_INPUT,
    INTERNAL_ERR,
};

//! Constants for database errors
enum db_err_nos {
    //! First error should be UNKNOWN as it maps to zero and zero is weird
    DB_ERROR_UNKNOWN,
    DB_ERROR_INTERNAL,
    // Probably should be removed at some point and replaced with bad_input_err
    DB_ERROR_BADINPUT,
    DB_ERROR_NOTFOUND,
    DB_ERROR_NOTIMPLEMENTED,
    DB_ERROR_DBCORRUPTED,
    DB_ERROR_NOTHINGINSERTED,
};

//! Constants for bad input type of error
enum bad_input_err {
    //! First error should be UNKNOWN as it maps to zero and zero is weird
    BAD_INPUT_UNKNOWN,
    BAD_INPUT_WRONG_INPUT,
    BAD_INPUT_OUT_OF_BOUNDS,
};

//! Constants for internal errors
enum internal_err {
    //! First error should be UNKNOWN as it maps to zero and zero is weird
    INTERNAL_UNKNOWN,
    INTERNAL_NOT_IMPLEMENTED
};

#endif // SRC_INCLUDE_DEFS_H_
