#ifndef SRC_INCLUDE_STR_DEFS_H__
#define SRC_INCLUDE_STR_DEFS_H__

// Archaisms, to be removed
extern const char* DRIVER_NMAP_SOCK;
extern const char* DRIVER_NMAP_REPLY;

// bios malamute endpoint
extern const char* MLM_ENDPOINT;

// names of malamute clients (== bios agents)
extern const char* BIOS_AGENT_NAME_COMPUTATION;
extern const char* BIOS_AGENT_PREFIX_REST; // each client created inside tntnet is suffixed by it's pid (getpid ()).
extern const char* BIOS_AGENT_NAME_DB_MEASUREMENT;

#define AVG_STEPS_SIZE 5
extern const char* AVG_STEPS[AVG_STEPS_SIZE];
#define AVG_TYPES_SIZE 3
extern const char* AVG_TYPES[AVG_TYPES_SIZE];

extern const char* DATETIME_FORMAT;
#define DATETIME_FORMAT_LENGTH 15

extern const char* STRFTIME_DATETIME_FORMAT;

// protocol related
extern const char* BIOS_WEB_AVERAGE_REPLY_JSON_TMPL;
extern const char* BIOS_WEB_AVERAGE_REPLY_JSON_DATA_ITEM_TMPL;

#endif // SRC_INCLUDE_STR_DEFS_H__

