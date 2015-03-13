#ifndef SRC_INCLUDE_DEFS_H__
#define SRC_INCLUDE_DEFS_H__

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
    DB_ERROR_CANTCONNECT,
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


//! netmon (rtnetlink(7)) events 
enum network_event {
    AUTO_ADD,
    AUTO_DEL,
    MAN_ADD,
    MAN_DEL,
    EXCL_ADD,
    EXCL_DEL
};

//! ip address version
enum ipaddr_version {
    IP_VERSION_4,
    IP_VERSION_6
};

//! Time interval in which measurement will be send regardless of difference
//TODO: Make it configurable
#define WORST_SAMPLING_INTERVAL 300

#define KEY_REPEAT "repeat"
#define KEY_STATUS "status"
#define KEY_CONTENT_TYPE "content-type"
#define PREFIX_X "x-"
#define OK      "ok"
#define ERROR   "error"
#define YES     "yes"
#define NO      "no"
#define KEY_ADD_INFO      "add_info"
#define KEY_AFFECTED_ROWS "affected_rows"
#define KEY_ERROR_TYPE     "error_type"
#define KEY_ERROR_SUBTYPE  "error_subtype"
#define KEY_ERROR_MSG      "error_msg"
#define KEY_ROWID          "rowid"

// netmon agent
#define NETMON_KEY_EVENT        "event"
#define NETMON_KEY_IFNAME       "interface name"
#define NETMON_KEY_IPVER        "ip version"
#define NETMON_KEY_IPADDR       "ip address"
#define NETMON_KEY_PREFIXLEN    "prefix length"
#define NETMON_KEY_MACADDR      "mac address"

#define NETMON_VAL_AUTO_ADD     "automatic addition"
#define NETMON_VAL_AUTO_DEL     "automatic deletion"
#define NETMON_VAL_MAN_ADD      "manual addition"
#define NETMON_VAL_MAN_DEL      "manual deletion"
#define NETMON_VAL_EXCL_ADD     "exclusion addition"
#define NETMON_VAL_EXCL_DEL     "exclusion deletion"

#define NETMON_VAL_IPV4     "ip version 4"
#define NETMON_VAL_IPV6     "ip version 6"

#endif // SRC_INCLUDE_DEFS_H__

