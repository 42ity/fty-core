/*
 *
 * Copyright (C) 2015 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*!
 * \file cleanup.h
 * \author Karol Hrdina
 * \brief Not yet documented file
 */
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

static inline void _destroy_byte (byte **self_p) {
    assert (self_p);
    if (*self_p) {
        free (*self_p);
        *self_p = NULL;
    }
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
#endif

#define _scoped_zactor_t    _cleanup_(_destroy_zactor) zactor_t
#define _scoped_zchunk_t    _cleanup_(_destroy_zchunk) zchunk_t
#define _scoped_zframe_t    _cleanup_(_destroy_zframe) zframe_t
#define _scoped_zpoller_t   _cleanup_(_destroy_zpoller) zpoller_t
#define _scoped_zrex_t      _cleanup_(_destroy_zrex) zrex_t
#define _scoped_zsock_t     _cleanup_(_destroy_zsock) zsock_t
#define _scoped_zhash_t     _cleanup_(_destroy_zhash) zhash_t
#define _scoped_zlist_t     _cleanup_(_destroy_zlist) zlist_t
#define _scoped_char        _cleanup_(_destroy_char) char 
#define _scoped_byte        _cleanup_(_destroy_byte) byte
#define _scoped_mlm_client_t        _cleanup_(_destroy_mlm_client) mlm_client_t

#endif // SRC_INCLUDE_CLEANUP_H__

