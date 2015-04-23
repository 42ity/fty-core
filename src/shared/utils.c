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

/*
Author(s): Michal Vyskocil <michalvyskocil@eaton.com>
           Karol Hrdina <karolhrdina@eaton.com>
           Tomas Halman <tomashalman@eaton.com>
 
Description: various random C and project wide helpers
*/

#include "utils.h"
#include "str_defs.h"

#include<limits.h>
#include<errno.h>
#include<stdlib.h>

const char *str_bool(const bool b) {
    return b ? "true" : "false";
}

const char *safe_str(const char *s) {
    return s == NULL ? "(null)" : s;
}

bool str_eq(const char *a, const char *b) {
	if (!a || !b) {
		return (!a && !b);
	}
	return !strcmp(a, b);
}

bool is_average_step_supported (const char *step) {
    if (!step) {
        return false;
    }
    for (int i = 0; i < AVG_STEPS_SIZE; ++i) {
        if (strcmp (step, AVG_STEPS[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool is_average_type_supported (const char *type) {
    if (!type) {
        return false;
    }
    for (int i = 0; i < AVG_TYPES_SIZE; ++i) {
        if (strcmp (type, AVG_TYPES[i]) == 0) {
            return true;
        }
    }
    return false;
}

int64_t string_to_int64( const char *value )
{
    char *end;
    int64_t result;
    errno = 0;
    if( ! value ) { errno = EINVAL; return INT64_MAX; }
    result = strtoll( value, &end, 10 );
    if( *end ) errno = EINVAL;
    if( errno ) return INT64_MAX;
    return result;
}

int32_t string_to_int32( const char *value )
{
    char *end;
    int32_t result;
    errno = 0;
    if( ! value ) { errno = EINVAL; return INT32_MAX; }
    result = strtol( value, &end, 10 );
    if( *end ) errno = EINVAL;
    if( errno ) return INT32_MAX;
    return result;
}
    
uint64_t string_to_uint64( const char *value )
{
    char *end;
    uint64_t result;
    errno = 0;
    if( ! value ) { errno = EINVAL; return UINT64_MAX; }
    result = strtoull( value, &end, 10 );
    if( *end ) errno = EINVAL;
    if( errno ) return UINT64_MAX;
    return result;
}

uint32_t string_to_uint32( const char *value )
{
    char *end;
    uint32_t result;
    errno = 0;
    if( ! value ) { errno = EINVAL; return UINT32_MAX; }
    result = strtoul( value, &end, 10);
    if( *end ) errno = EINVAL;
    if( errno ) { return UINT32_MAX; }
    return result;
}

