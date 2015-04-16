#include "str_defs.h"

/* Implement variables declared in str_defs.h */
/* NOTE: In header file the declaration used to be static const, now it is extern const. In source file: const in .c */
const char* DRIVER_NMAP_SOCK = "ipc://@/bios/driver/nmap";
const char* DRIVER_NMAP_REPLY = "ipc://@/bios/driver/nmap_reply";

const char* MLM_ENDPOINT = "ipc://@/malamute";

const char *BIOS_AGENT_NAME_COMPUTATION = "cm-agent";
const char *BIOS_AGENT_PREFIX_REST = "rest.";
const char *BIOS_AGENT_NAME_DB_MEASUREMENT = "persistence.measurement";

const char *AVG_STEPS[AVG_STEPS_SIZE] = {
    "15m",
    "30m",
    "1h",
    "8h",
    "24h"
};

const char *AVG_TYPES[AVG_TYPES_SIZE] = {
    "arithmetic_mean",
    "min",
    "max"
};
