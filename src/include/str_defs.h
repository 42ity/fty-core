#ifndef SRC_INCLUDE_STR_DEFS_H__
#define SRC_INCLUDE_STR_DEFS_H__

/* Implemented in defs.c, link with that if you use any of these values */
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

#endif // SRC_INCLUDE_STR_DEFS_H__

