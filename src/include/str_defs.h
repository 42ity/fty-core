#ifndef SRC_INCLUDE_STR_DEFS_H__
#define SRC_INCLUDE_STR_DEFS_H__

/* Implemented in defs.c, link with that if you use any of these values */

//TODO (jim): When are we going to remove these five string constants? Are we going to keep it balsamed just like simple? Are we creating a mausoleum here? ;)
extern const char* DB_SOCK;
extern const char* FILIP_SOCK;
extern const char* CLI_SOCK;

extern const char* DRIVER_NMAP_SOCK;
extern const char* DRIVER_NMAP_REPLY;

extern const char* MLM_ENDPOINT;

#define AVG_STEPS_SIZE 5
extern const char *AVG_STEPS[AVG_STEPS_SIZE];

#endif // SRC_INCLUDE_STR_DEFS_H__

