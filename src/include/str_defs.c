#include "str_defs.h"

/* Implement variables declared in str_defs.h */
/* NOTE: In header file the declaration used to be static const, now it is extern const. In source file: const in .c */
const char* DB_SOCK = "ipc://@/poord/persistence";
const char* FILIP_SOCK = "ipc://@/poord/netlogic";
const char* CLI_SOCK = "ipc://@/poord/cli";

const char* DRIVER_NMAP_SOCK = "ipc://@/bios/driver/nmap";
const char* DRIVER_NMAP_REPLY = "ipc://@/bios/driver/nmap_reply";
const char* MLM_ENDPOINT = "ipc://@/malamute";

const char *AVG_STEPS[AVG_STEPS_SIZE] = {
    "8h",
    "16h",
    "1d",
    "1w",
    "1m"
};
