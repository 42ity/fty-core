/* 
Copyright (C) 2014 - 2015 Eaton
 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/*! \file utils_ymsg++.h
    \brief Utility functions for ymsg_t that we don't want to expose as public API in libbiosapi
    \author Karol Hrdina <KarolHrdina@Eaton.com>
*/

#ifndef SRC_SHARED_UTILS_YMSG_H__
#define SRC_SHARED_UTILS_YMSG_H__

#include "ymsg.h"
#include "app.h"

#ifdef __cplusplus
extern "C"
{
#endif

// TODO write doxygen
// These misuse conversion to/from string to transfer numbers around    
// I have a better set prepared, coming soon ;)
int
    ymsg_aux_uint32 (ymsg_t *self, const char *key, uint32_t *value);
int
    ymsg_aux_uint64 (ymsg_t *self, const char *key, uint64_t *value);
int
    ymsg_aux_int32 (ymsg_t *self, const char *key, int32_t *value);
int
    ymsg_aux_int64 (ymsg_t *self, const char *key, int64_t *value);
int
    ymsg_aux_double (ymsg_t *self, const char *key, double *value);
void
    ymsg_aux_set_uint32 (ymsg_t *self, const char *key, uint32_t value);
void
    ymsg_aux_set_uint64 (ymsg_t *self, const char *key, uint64_t value);
void
    ymsg_aux_set_int32 (ymsg_t *self, const char *key, int32_t value);
void
    ymsg_aux_set_int64 (ymsg_t *self, const char *key, int64_t value);
void
    ymsg_aux_set_double (ymsg_t *self, const char *key, uint8_t precision, double value);
int
    ymsg_request_set_app (ymsg_t *self, app_t **request);
app_t *
    ymsg_request_app(ymsg_t *ymsg);
int
    ymsg_response_set_app (ymsg_t *self, app_t **response);
app_t *
    ymsg_response_app(ymsg_t *ymsg);


// TODO ACE: write documentation
int
    ymsg_rowid (ymsg_t *self, uint64_t *value);
void
    ymsg_set_rowid (ymsg_t *self, uint64_t rowid);
int32_t
    ymsg_errtype (ymsg_t *self);
void
    ymsg_set_errtype (ymsg_t *self, int32_t error_type);
int32_t
    ymsg_errsubtype (ymsg_t *self);
void
    ymsg_set_errsubtype (ymsg_t *self, int32_t error_subtype);
const char*
    ymsg_errmsg (ymsg_t *self);
void
    ymsg_set_errmsg (ymsg_t *self, const char *error_msg);

// without ownership transfer
zhash_t*
    ymsg_addinfo (ymsg_t *self);
// transfer ownership
zhash_t*
    ymsg_get_addinfo (ymsg_t *self);
int
    ymsg_set_addinfo (ymsg_t *self, zhash_t *addinfo);
ymsg_t*
    ymsg_generate_ok(uint64_t rowid, zhash_t *addinfo);
ymsg_t*
    ymsg_generate_fail (int errtype, int errsubtype, const char *errmsg, zhash_t *addinfo);

/*!
 \brief Get number of rows affected of ROZP REPLY message
 \return  number of rows affected. If key is not specified, then -1.
*/
int64_t
    ymsg_affected_rows (ymsg_t *self);

/*!
 \brief Set number of rows affected in ROZP REPLY message
*/
void
    ymsg_set_affected_rows (ymsg_t *self, int64_t n);

#ifdef __cplusplus
}
#endif

#endif // SRC_SHARED_UTILS_YMSG_H__    

