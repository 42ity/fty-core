#include "str_defs.h"

/* Implement variables declared in str_defs.h */
/* NOTE: In header file the declaration used to be static const, now it is extern const. In source file: const in .c */
const char* DRIVER_NMAP_SOCK = "ipc://@/bios/driver/nmap";
const char* DRIVER_NMAP_REPLY = "ipc://@/bios/driver/nmap_reply";

const char* MLM_ENDPOINT = "ipc://@/malamute";

const char *BIOS_AGENT_NAME_COMPUTATION = "agent-cm";
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

const char* DATETIME_FORMAT = "%4d%2d%2d%2d%2d%2d%c";

const char* BIOS_WEB_AVERAGE_REPLY_JSON_TMPL =
    "{\n"
    "\"units\": \"##UNITS##\",\n"
    "\"source\": \"##SOURCE##\",\n"
    "\"step\": \"##STEP##\",\n"
    "\"type\": \"##TYPE##\",\n"
    "\"element_id\": ##ELEMENT_ID##,\n"
    "\"start_ts\": ##START_TS##,\n"
    "\"end_ts\": ##END_TS##,\n"
    "\"data\": [\n"
    "##DATA##\n"
    "]}";
const char* BIOS_WEB_AVERAGE_REPLY_JSON_DATA_ITEM_TMPL =
    "\t{\n"
    "\t\t\"value\": ##VALUE##,\n"
    "\t\t\"timestamp\": ##TIMESTAMP##\n"
    "\t}";

const char* STRFTIME_DATETIME_FORMAT = "%Y%m%d%H%M%SZ";    

