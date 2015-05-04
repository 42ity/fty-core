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
    char *substr = strndup (step, strlen (step) - 1);
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
        return timestamp;
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


int unixtime_to_datetime(time_t t, char* buf, size_t s) {
    struct tm* tmp = gmtime(&t);
    if (!tmp)
        return -1;
    strftime(buf, s, "%Y%m%d%H%M%SZ", tmp);
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

