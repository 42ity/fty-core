#include "defs.h"

/* Implement variables declared in defs.h */
/* NOTE: were static const, now extern const in .h, const in .c */
const char* DB_SOCK = "ipc://@/poord/persistence";
const char* FILIP_SOCK = "ipc://@/poord/netlogic";
const char* CLI_SOCK = "ipc://@/poord/cli";

const char* DRIVER_NMAP_SOCK = "ipc://@/bios/driver/nmap";
const char* DRIVER_NMAP_REPLY = "ipc://@/bios/driver/nmap_reply";
const char* MLM_ENDPOINT = "ipc://@/malamute";
