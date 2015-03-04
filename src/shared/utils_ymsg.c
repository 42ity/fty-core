/* 
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
#include "utils_ymsg.h"
#include "defs.h"

int
ymsg_aux_unsigned (ymsg_t *self, const char *key, uint64_t *value)
{
    if (!self || !key || !value || !ymsg_aux(self)) {
        return -1;
    }

    char *string = NULL;
    string = (char *) (zhash_lookup (ymsg_aux(self), key));
    if (!string) {
        return -1;
    }

    char *end;
    uint64_t u = (uint64_t) strtoull (string, &end, 10);
    if (errno == ERANGE) {
        errno = 0;
        return -1;
    }
    else if (end == string || *end != '\0') {
        return -1;
    }
    *value = u;
    return 0;
}

int
ymsg_aux_signed (ymsg_t *self, const char *key, int64_t *value) {
    if (!self || !key || !value || !ymsg_aux(self)) {
        return -1;
    }

    char *string = NULL;
    string = (char *) (zhash_lookup (ymsg_aux (self), key));
    if (!string) {
        return -1;
    }

    char *end;
    int64_t u = (int64_t) strtoll (string, &end, 10);
    if (errno == ERANGE) {
        errno = 0;
        return -1;
    }
    else if (end == string || *end != '\0') {
        return -1;
    }
    *value = u;
    return 0;
}


int
ymsg_aux_double (ymsg_t *self, const char *key, double *value) {
    if (!self || !key || !value || !ymsg_aux(self)) {
        return -1;
    }

    char *string = NULL;
    string = (char *) (zhash_lookup (ymsg_aux (self), key));
    if (!string) {
        return -1;
    }

    char *end;
    double u = strtod (string, &end);
    if (errno == ERANGE) {
        errno = 0;
        return -1;
    }
    else if (end == string || *end != '\0') {
        return -1;
    }
    *value = u;
    return 0;
}

void
ymsg_aux_set_unsigned (ymsg_t *self, const char *key, uint64_t value) {
    if (!self || !key) {
        return;
    }
    ymsg_aux_insert (self, key, "%"PRIu64, value);
}

void
ymsg_aux_set_signed (ymsg_t *self, const char *key, int64_t value) {
    if (!self || !key) {
        return;
    }
    ymsg_aux_insert (self, key, "%"PRId64, value);
}

void
ymsg_aux_set_double (ymsg_t *self, const char *key, uint8_t precision, double value) {
    if (!self || !key) {
        return;
    }
    ymsg_aux_insert (self, key, "%.*lf", precision, value);
}

int
ymsg_rowid (ymsg_t *self, uint64_t *value) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return -1;
    }
    return ymsg_aux_unsigned (self, KEY_ROWID, value);
}

void
ymsg_set_rowid (ymsg_t *self, uint64_t rowid) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return;
    }
    ymsg_aux_set_unsigned (self, KEY_ROWID, rowid);
}

int
ymsg_errtype (ymsg_t *self) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return -1;
    }
    uint64_t ui;
    int rc =  ymsg_aux_unsigned (self, KEY_ERROR_TYPE, &ui);
    if (rc != 0 || ui > INT_MAX)  {
        return -1;
    }
    return (int) ui;
}

void
ymsg_set_errtype (ymsg_t *self, int error_type) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return;
    }
    ymsg_aux_set_unsigned (self, KEY_ERROR_TYPE, error_type);
}

int
ymsg_errsubtype (ymsg_t *self) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return -1;
    }
    uint64_t ui;
    int rc =  ymsg_aux_unsigned (self, KEY_ERROR_SUBTYPE, &ui);
    if (rc != 0 || ui > INT_MAX)  {
        return -1;
    }
    return (int) ui;
}

void
ymsg_set_errsubtype (ymsg_t *self, int error_subtype) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return;
    }
    ymsg_aux_set_unsigned (self, KEY_ERROR_SUBTYPE, error_subtype);
}

const char*
ymsg_errmsg (ymsg_t *self) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return NULL;
    }
    return ymsg_aux_string (self, KEY_ERROR_MSG, NULL);
}

void
ymsg_set_errmsg (ymsg_t *self, const char *error_msg) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return;
    }
    ymsg_aux_insert (self, KEY_ERROR_MSG, "%s", error_msg);
}

zhash_t*
ymsg_addinfo (ymsg_t *self) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return NULL;
    }
    // Note: I don't know what Alenka meant here
    // TODO
    return NULL;
}

zhash_t*
ymsg_get_addinfo (ymsg_t *self) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return NULL;
    }
    // Note: I don't know what Alenka meant here
    // TODO
    return NULL;
}

int
ymsg_set_addinfo (ymsg_t *self, zhash_t *addinfo) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return -1;
    }
    // Note: I don't know what Alenka meant here
    // TODO
    return 0;
}

ymsg_t*
ymsg_generate_ok(uint64_t rowid, zhash_t *addinfo)
{
    ymsg_t *resultmsg = ymsg_new (YMSG_REPLY);
    ymsg_set_rowid (resultmsg, rowid);
    if ( addinfo != NULL )
        ymsg_set_addinfo (resultmsg, addinfo);
    return resultmsg;
}

ymsg_t*
ymsg_generate_fail (int errtype, int errsubtype, const char *errmsg, zhash_t *addinfo)
{
    ymsg_t* resultmsg = ymsg_new (YMSG_REPLY);
    ymsg_set_errtype    (resultmsg, errtype);
    ymsg_set_errsubtype (resultmsg, errsubtype);
    ymsg_set_errmsg     (resultmsg, errmsg );
    if ( addinfo != NULL )
        ymsg_set_addinfo  (resultmsg, addinfo);
    return resultmsg;
}

int
ymsg_affected_rows (ymsg_t *self)
{
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return -1;
    }
    uint64_t ui;
    int rc =  ymsg_aux_unsigned (self, KEY_AFFECTED_ROWS, &ui);
    if (rc != 0 || ui > INT_MAX)  {
        return -1;
    }
    return (int) ui;
}

void
ymsg_set_affected_rows (ymsg_t *self, uint64_t n)
{
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return;
    }
    ymsg_aux_set_unsigned (self, KEY_AFFECTED_ROWS, n);
}




