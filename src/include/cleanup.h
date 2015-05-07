#ifndef SRC_INCLUDE_CLEANUP_H__
#define SRC_INCLUDE_CLEANUP_H__

// TODO: Later we will move this to configure
#define LEGACY_PROTOCOL 1

#include <czmq.h>
#include <malamute.h>
#include "ymsg.h"
#include "app.h"
#include "bios_agent.h"

#ifdef LEGACY_PROTOCOL
#include "asset_msg.h"
#include "common_msg.h"
#include "compute_msg.h"
#include "netdisc_msg.h"
#include "nmap_msg.h"
#include "powerdev_msg.h"
#endif

#ifndef __GNUC__
#error Please use a compiler supporting attribute cleanup
#endif

static inline void _destroy_zmsg (zmsg_t **self_p) {
    zmsg_destroy (self_p);
}
static inline void _destroy_ymsg (ymsg_t **self_p) {
    ymsg_destroy (self_p);
}

static inline void _destroy_app (app_t **self_p) {
    app_destroy (self_p);
}

static inline void _destroy_bios_agent (bios_agent_t **self_p) {
    bios_agent_destroy (self_p);
}

#ifdef LEGACY_PROTOCOL
static inline void _destroy_asset_msg (asset_msg_t **self_p) {
    asset_msg_destroy (self_p);
}

static inline void _destroy_common_msg (common_msg_t **self_p) {
    common_msg_destroy (self_p);
}

static inline void _destroy_compute_msg (compute_msg_t **self_p) {
    compute_msg_destroy (self_p);
}

static inline void _destroy_netdisc_msg (netdisc_msg_t **self_p) {
    netdisc_msg_destroy (self_p);
}

static inline void _destroy_nmap_msg (nmap_msg_t **self_p) {
    nmap_msg_destroy (self_p);
}

static inline void _destroy_powerdev_msg (powerdev_msg_t **self_p) {
    powerdev_msg_destroy (self_p);
}
#endif

static inline void _destroy_zactor (zactor_t **self_p) {
    zactor_destroy (self_p);
}

static inline void _destroy_zchunk (zchunk_t **self_p) {
    zchunk_destroy (self_p);
}


static inline void _destroy_zframe (zframe_t **self_p) {
    zframe_destroy (self_p);
}

static inline void _destroy_zpoller (zpoller_t **self_p) {
    zpoller_destroy (self_p);
}

static inline void _destroy_zrex (zrex_t **self_p) {
    zrex_destroy (self_p);
}

static inline void _destroy_zsock (zsock_t **self_p) {
    zsock_destroy (self_p);
}


static inline void _destroy_zhash (zhash_t **self_p) {
    zhash_destroy (self_p);
}

static inline void _destroy_zlist (zlist_t **self_p) {
    zlist_destroy (self_p);
}

static inline void _destroy_char (char **self_p) {
    zstr_free (self_p);
}

static inline void _destroy_mlm_client (mlm_client_t **self_p) {
    mlm_client_destroy (self_p);
}

#define _cleanup_(x) __attribute__((cleanup(x)))

#define _scoped_zmsg_t          _cleanup_(_destroy_zmsg) zmsg_t
#define _scoped_ymsg_t          _cleanup_(_destroy_ymsg) ymsg_t
#define _scoped_app_t           _cleanup_(_destroy_app) app_t
#define _scoped_bios_agent_t    _cleanup_(_destroy_bios_agent) bios_agent_t

#ifdef LEGACY_PROTOCOL
#define _scoped_asset_msg_t     _cleanup_(_destroy_asset_msg) asset_msg_t
#define _scoped_common_msg_t    _cleanup_(_destroy_common_msg) common_msg_t
#define _scoped_compute_msg_t   _cleanup_(_destroy_compute_msg) compute_msg_t
#define _scoped_netdisc_msg_t   _cleanup_(_destroy_netdisc_msg) netdisc_msg_t
#define _scoped_nmap_msg_t      _cleanup_(_destroy_nmap_msg) nmap_msg_t
#define _scoped_powerdev_msg_t  _cleanup_(_destroy_powerdev_msg) powerdev_msg_t
#endif

#define _scoped_zactor_t    _cleanup_(_destroy_zactor) zactor_t
#define _scoped_zchunk_t    _cleanup_(_destroy_zchunk) zchunk_t
#define _scoped_zframe_t    _cleanup_(_destroy_zframe) zframe_t
#define _scoped_zpoller_t   _cleanup_(_destroy_zpoller) zpoller_t
#define _scoped_zrex_t      _cleanup_(_destroy_zrex) zrex_t
#define _scoped_zsock_t     _cleanup_(_destroy_zsock) zsock_t
#define _scoped_zhash_t     _cleanup_(_destroy_zhash) zhash_t
#define _scoped_zlist_t     _cleanup_(_destroy_zlist) zlist_t
#define _scoped_char        _cleanup_(_destroy_char) char // this is a better "version" of FREE0 macro 
#define _scoped_mlm_client_t        _cleanup_(_destroy_mlm_client) mlm_client_t
// TODO: byte

#endif // SRC_INCLUDE_CLEANUP_H__

