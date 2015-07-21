#ifndef SRC_INCLUDE_STR_DEFS_H__
#define SRC_INCLUDE_STR_DEFS_H__

// bios malamute endpoint
extern const char* MLM_ENDPOINT;

// agent netmon name and subject
extern const char* BIOS_AGENT_NAME_NETMON;

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

// evironment variables
extern const char* EV_BIOS_LOG_LEVEL;

#endif // SRC_INCLUDE_STR_DEFS_H__

