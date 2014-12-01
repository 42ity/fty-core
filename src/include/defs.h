#ifndef SRC_INCLUDE_DEFS_H_
#define SRC_INCLUDE_DEFS_H_

// marker to tell humans and GCC that the unused parameter is there for some
// reason (i.e. API compatibility) and compiler should not warn if not used
#if !defined(UNUSED_PARAM) && defined (__GNUC__)
# define UNUSED_PARAM __attribute__ ((__unused__))
#else
# define UNUSED_PARAM
#endif

static const char* DB_SOCK = "ipc://@/poord/persistence";
static const char* FILIP_SOCK = "ipc://@/poord/netlogic";
static const char* CLI_SOCK = "ipc://@/poord/cli";

static const char* DRIVER_NMAP_SOCK = "ipc://@/bios/driver/nmap";
static const char* DRIVER_NMAP_REPLY = "ipc://@/bios/driver/nmap_reply";

#endif // SRC_INCLUDE_DEFS_H_

