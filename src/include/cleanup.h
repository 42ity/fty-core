#ifndef SRC_INCLUDE_CLEANUP_H__
#define SRC_INCLUDE_CLEANUP_H__

#include <czmq.h>
#include "ymsg.h"
#include "app.h"
#include "bios_agent.h"

#ifndef __GNUC__
#error Use the real compiler, Luke
#endif

static inline void _destroy_zmsg (zmsg_t **msg) {
    zmsg_destroy (msg);
}
static inline void _destroy_ymsg (ymsg_t **msg) {
    ymsg_destroy (msg);
}

static inline void _destroy_app (app_t **msg) {
    app_destroy (msg);
}

static inline void _destroy_bios_agent (bios_agent_t **msg) {
    bios_agent_destroy (msg);
}

#define _cleanup_(x) __attribute__((cleanup(x)))

#define _scoped_zmsg_t  _cleanup_(_destroy_zmsg)    zmsg_t
#define _scoped_ymsg_t  _cleanup_(_destroy_ymsg)    ymsg_t
#define _scoped_app_t   _cleanup_(_destroy_app)     app_t
#define _scoped_bios_agent_t   _cleanup_(_destroy_bios_agent)     bios_agent_t

#endif // SRC_INCLUDE_CLEANUP_H__

