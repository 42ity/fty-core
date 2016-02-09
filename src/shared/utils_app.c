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
 * \file utils_app.c
 * \author Tomas Halman <TomasHalman@Eaton.com>
 * \author Jim Klimov <EvgenyKlimov@Eaton.com>
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \brief Not yet documented file
 */
#include "utils_app.h"
#include "cleanup.h"
#include "utils.h"

int32_t
app_params_first_int32(app_t* msg) {
    errno = 0;
    if( ! msg ) { errno = EINVAL; return INT32_MAX; }
    return string_to_int32( app_params_first(msg) );
}

int64_t
app_params_first_int64(app_t* msg) {
    errno = 0;
    if( ! msg ) { errno = EINVAL; return INT64_MAX; }
    return string_to_int64( app_params_first(msg) );
}

uint32_t
app_params_first_uint32(app_t* msg) {
    errno = 0;
    if( ! msg ) { errno = EINVAL; return UINT32_MAX; }
    return string_to_uint32( app_params_first(msg) );
}

uint64_t
app_params_first_uint64(app_t* msg)
{
    errno = 0;
    if( ! msg ) { errno = EINVAL; return UINT64_MAX; }
    return string_to_uint64( app_params_first(msg) );
}

int32_t
app_params_next_int32(app_t* msg)
{
    errno = 0;
    if( ! msg ) { errno = EINVAL; return INT32_MAX; }
    return string_to_int32( app_params_next(msg) );
}

int64_t
app_params_next_int64(app_t* msg)
{
    errno = 0;
    if( ! msg ) { errno = EINVAL; return INT64_MAX; }
    return string_to_int64( app_params_next(msg) );
}

uint32_t
app_params_next_uint32(app_t* msg)
{
    errno = 0;
    if( ! msg ) { errno = EINVAL; return UINT32_MAX; }
    return string_to_uint32( app_params_next(msg) );
}

uint64_t
app_params_next_uint64(app_t* msg)
{
    errno = 0;
    if( ! msg ) { errno = EINVAL; return UINT64_MAX; }
    return string_to_uint64( app_params_next(msg) );
}

void
app_params_append_string(app_t* msg, const char *value)
{
    if(!msg) { errno = EINVAL; return ; };

    zlist_t *list = app_get_params(msg);
    if(list == NULL) {
        list = zlist_new();
        zlist_autofree(list);
    }
    zlist_append( list, (void*) strdup (value) );
    app_set_params( msg, &list );
}

void
app_params_append_int64(app_t* msg, int64_t value )
{
    if(!msg) { errno = EINVAL; return; };

    char buff[24];
    memset( buff, '\0', sizeof(buff) );
    snprintf( buff, sizeof(buff)-1, "%" PRIi64, value );
    app_params_append_string( msg, buff );
}

void
app_params_append_uint64(app_t* msg, uint64_t value )
{
    if(!msg) { errno = EINVAL; return; };

    char buff[24];
    memset( buff, '\0', sizeof(buff) );
    snprintf( buff, sizeof(buff)-1, "%" PRIu64, value );
    app_params_append_string( msg, buff );
}


void
app_params_append_int32(app_t* msg, int32_t value )
{
    app_params_append_int64( msg, value );
}

void
app_params_append_uint32(app_t* msg, uint32_t value )
{
    app_params_append_uint64( msg, value );
}


/* -------- args ------------ */
void
app_args_set_string(app_t* msg, const char *key, const char *value) {
    errno = 0;
    if(!msg) { errno = EINVAL; return ; };

    _scoped_zhash_t *hash = app_get_args(msg);
    if(hash == NULL) {
        hash = zhash_new();
        zhash_autofree(hash);
    }
    zhash_update(hash, key, (void*) strdup (value));
    app_set_args(msg, &hash);
}

void
app_args_set_int64(app_t* msg, const char *key, int64_t value ) {
    errno = 0;
    if(!msg) { errno = EINVAL; return; };

    char buff[24];
    memset( buff, '\0', sizeof(buff) );
    snprintf( buff, sizeof(buff)-1, "%" PRIi64, value );
    app_args_set_string( msg, key, buff );
}

void
app_args_set_uint64(app_t* msg, const char *key, uint64_t value ) {
    errno = 0;
    if(!msg) { errno = EINVAL; return; };

    char buff[24];
    memset( buff, '\0', sizeof(buff) );
    snprintf( buff, sizeof(buff)-1, "%" PRIu64, value );
    app_args_set_string( msg, key, buff );
}

int8_t
app_args_int8(app_t* msg, const char *key) {
    if(!msg) { errno = EINVAL; return INT8_MAX; };

    const char *val = app_args_string( msg, key, NULL );
    if(val == NULL) {
        errno = EKEYREJECTED;
        return INT8_MAX;
    }
    return string_to_int8(val);
}

int16_t
app_args_int16(app_t* msg, const char *key) {
    if(!msg) { errno = EINVAL; return INT16_MAX; };

    const char *val = app_args_string( msg, key, NULL );
    if(val == NULL) {
        errno = EKEYREJECTED;
        return INT16_MAX;
    }
    return string_to_int16(val);
}

int32_t
app_args_int32(app_t* msg, const char *key) {
    if(!msg) { errno = EINVAL; return INT32_MAX; };

    const char *val = app_args_string( msg, key, NULL );
    if(val == NULL) {
        errno = EKEYREJECTED;
        return INT32_MAX;
    }
    return string_to_int32(val);
}

int64_t
app_args_int64(app_t* msg, const char *key) {
    if(!msg) { errno = EINVAL; return INT64_MAX; };

    const char *val = app_args_string( msg, key, NULL );
    if(val == NULL) {
        errno = EKEYREJECTED;
        return INT64_MAX;
    }
    return string_to_int64(val);
}

uint8_t
app_args_uint8(app_t* msg, const char *key) {
    if(!msg) { errno = EINVAL; return UINT8_MAX; };

    const char *val = app_args_string( msg, key, NULL );
    if(val == NULL) {
        errno = EKEYREJECTED;
        return UINT8_MAX;
    }
    return string_to_uint8(val);
}

uint16_t
app_args_uint16(app_t* msg, const char *key) {
    if(!msg) { errno = EINVAL; return UINT16_MAX; };

    const char *val = app_args_string( msg, key, NULL );
    if(val == NULL) {
        errno = EKEYREJECTED;
        return UINT16_MAX;
    }
    return string_to_uint16(val);
}

uint32_t
app_args_uint32(app_t* msg, const char *key) {
    if(!msg) { errno = EINVAL; return UINT32_MAX; };

    const char *val = app_args_string( msg, key, NULL );
    if(val == NULL) {
        errno = EKEYREJECTED;
        return UINT32_MAX;
    }
    return string_to_uint32(val);
}

uint64_t
app_args_uint64(app_t* msg, const char *key) {
    if(!msg) { errno = EBADMSG; return UINT64_MAX; };

    const char *val = app_args_string( msg, key, NULL );
    if(val == NULL) {
        errno = EKEYREJECTED;
        return UINT64_MAX;
    }
    return string_to_uint64(val);
}
