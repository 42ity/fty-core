/* utils_ymsg.h: TODO
 
Copyright (C) 2014 - 2015 Eaton
 
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
Author(s): Karol Hrdina <karolhrdina@eaton.com>
 
Description: TODO
*/

#ifndef SRC_SHARED_UTILS_YMSG__
#define SRC_SHARED_UTILS_YMSG__

#include "ymsg.h"

#ifdef __cplusplus
extern "C"
{
#endif

// TODO write doxygen
// These misuse conversion to/from string to transfer numbers around    
// I have a better set prepared, coming soon ;)
int
    ymsg_aux_unsigned (ymsg_t *self, const char *key, uint64_t *value);
int
    ymsg_aux_signed (ymsg_t *self, const char *key, int64_t *value);
int
    ymsg_aux_double (ymsg_t *self, const char *key, double *value);
void
    ymsg_aux_set_unsigned (ymsg_t *self, const char *key, uint64_t value);
void
    ymsg_aux_set_signed (ymsg_t *self, const char *key, int64_t value);
void
    ymsg_aux_set_double (ymsg_t *self, const char *key, uint8_t precision, double value);

// TODO ACE: write documentation
int
    ymsg_rowid (ymsg_t *self, uint64_t *value);
void
    ymsg_set_rowid (ymsg_t *self, uint64_t rowid);
int
    ymsg_errtype (ymsg_t *self);
void
    ymsg_set_errtype (ymsg_t *self, int error_type);
int
    ymsg_errsubtype (ymsg_t *self);
void
    ymsg_set_errsubtype (ymsg_t *self, int error_subtype);
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
int
    ymsg_affected_rows (ymsg_t *self);

/*!
 \brief Set number of rows affected in ROZP REPLY message
*/
void
    ymsg_set_affected_rows (ymsg_t *self, uint64_t n);

#ifdef __cplusplus
}
#endif
#endif // SRC_SHARED_UTILS_YMSG__    
