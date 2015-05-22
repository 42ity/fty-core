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
 
Description: various random C and project wide helpers
*/
#include <assert.h>

#include "utils.h"
#include "str_defs.h"
#include "defs.h"
#include "cleanup.h"

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

int64_t average_step_seconds (const char *step) {
    if (!is_average_step_supported (step))
        return -1;
    // currently we are using m (minute) h (hour), d (day)
    int c = step [strlen (step) - 1];
    int multiplier = -1;
    switch (c) {
        case 109: // minute
            {
                multiplier = 60;
                break;
            }
        case 104: // hour
            {
                multiplier = 3600;
                break;
            }
        case 100: // day
            {
                multiplier = 86400;
                break;
            }
        default:
            return -1;
    }
    _scoped_char *substr = strndup (step, strlen (step) - 1);
    if (!substr)
        return -1;
    int number = atoi (substr);
    free (substr); substr = NULL;
    return (int64_t) (number * multiplier);
}

int64_t average_extend_left_margin (int64_t start_timestamp, const char *step) {
    if (!is_average_step_supported (step))
        return -1;
    int64_t step_sec = average_step_seconds (step);
    unsigned int overlap = start_timestamp % step_sec;
    int64_t result;
    if (overlap == 0)
        result =  (int64_t) start_timestamp - step_sec - AGENT_NUT_REPEAT_INTERVAL_SEC;
    else
        result =  (int64_t) start_timestamp - overlap - AGENT_NUT_REPEAT_INTERVAL_SEC;
    return result;
}

int64_t average_first_since (int64_t timestamp, const char *step) {
    if (!is_average_step_supported (step) || timestamp < 0)
        return -1;
    int64_t step_sec = average_step_seconds (step);
    int64_t mod = timestamp % step_sec;
    if (mod == 0)
        return timestamp + step_sec;
    return timestamp + (step_sec - mod);
}

int64_t datetime_to_calendar (const char *datetime) {
    if (!datetime || strlen (datetime) != DATETIME_FORMAT_LENGTH)
        return -1;
    int year, month, day, hour, minute, second;
    char suffix;
    int rv = sscanf (datetime, DATETIME_FORMAT, &year, &month, &day, &hour, &minute, &second, &suffix);
    if (rv != 7 || suffix != 'Z') {
        return -1;
    }
    struct tm tm;
    tm.tm_year =  year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;
    int64_t t = my_timegm (&tm);
    assert (tm.tm_isdst == 0);
    return t;
}


int calendar_to_datetime(time_t timestamp, char* buffer, size_t n) {
    struct tm* tmp = gmtime (&timestamp);
    if (!tmp || strftime (buffer, n, STRFTIME_DATETIME_FORMAT, tmp) == 0) { // it's safe to check for 0, since we expect non-zero string
        return -1;
    }
    return 0;
}

int64_t my_timegm (struct tm *tm) {
    // set the TZ environment variable to UTC, call mktime(3) and restore the value of TZ. 
    time_t ret;
    char *tz;
    tz = getenv("TZ");
    setenv("TZ", "", 1);
    tzset ();
    ret = mktime (tm);
    if (tz)
        setenv ("TZ", tz, 1);
    else
        unsetenv ("TZ");
    tzset ();
    return (int64_t) ret;
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

int16_t string_to_int16( const char *value )
{
    int32_t result = string_to_int32(value);
    if( result > INT16_MAX || result < INT16_MIN ) {
        errno = EINVAL;
        return INT16_MAX;
    }
    return (int16_t)result;
}

uint16_t string_to_uint16( const char *value )
{
    uint32_t result = string_to_uint32(value);
    if( result > UINT16_MAX ) {
        errno = EINVAL;
        return UINT16_MAX;
    }
    return (uint16_t)result;
}

int8_t string_to_int8( const char *value )
{
    int32_t result = string_to_int32(value);
    if( result > INT8_MAX || result < INT8_MIN ) {
        errno = EINVAL;
        return INT8_MAX;
    }
    return (int8_t)result;
}

uint8_t string_to_uint8( const char *value )
{
    uint32_t result = string_to_uint32(value);
    if( result > UINT8_MAX ) {
        errno = EINVAL;
        return UINT8_MAX;
    }
    return (uint8_t)result;
}

const char* alert_state_to_str(alert_state_t astate) {
    switch (astate) {
        case ALERT_STATE_UNKNOWN:
            return "unknown";;
        case ALERT_STATE_NO_ALERT:
            return "no-alert";;
        case ALERT_STATE_ONGOING_ALERT:
            return "ongoing";;
    }
    return "unknown"; //make gcc happy
}

bool addi32_overflow(int32_t a, int32_t b, int32_t *res) {
    int64_t l_res = (int64_t)a + (int64_t)b;
    if (l_res > INT32_MAX || l_res < INT32_MIN)
        return false;
    *res = (int32_t) l_res; //this is safe because of check above
    return true;
}

bool bsi32_rescale(int32_t in_value, int8_t in_scale, int8_t new_scale, int32_t* value)
{
    if (in_scale == new_scale) {
        *value = in_value;
        return true;
    }

    int32_t l_value = in_value;

    if (in_scale > new_scale) {
        for (int i = 0; i != abs(in_scale - new_scale); i++) {
            if (l_value >= (INT32_MAX / 10) || l_value <= (INT32_MIN / 10))
                return false;
            l_value *= 10;
        }
        *value = l_value;
        return true;
    }

    for (int i = 0; i != abs(in_scale - new_scale); i++) {
        l_value /= 10;
    }
    *value = l_value;
    return true;
}

bool bsi32_add(int32_t value1, int8_t scale1,
            int32_t value2, int8_t scale2,
            int32_t *value, int8_t* scale)
{
    bool ret = false;

    int32_t l_value = 0, l_value1 = 0, l_value2 = 0;
    int8_t l_scale = 0;

    l_scale = (scale1 < scale2) ? scale1 : scale2;

    ret = bsi32_rescale(value1, scale1, l_scale, &l_value1);
    if (!ret)
        return false;

    ret = bsi32_rescale(value2, scale2, l_scale, &l_value2);
    if (!ret)
        return false;

    ret = addi32_overflow(l_value1, l_value2, &l_value);
    if (!ret)
        return false;

    *value = l_value;
    *scale = l_scale;
    return true;
}
