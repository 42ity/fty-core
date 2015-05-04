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

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*! \brief return "true" or "false" for bool argument */
const char *str_bool(const bool b);

/*! \brief return "(null)" if string argument is NULL */
const char *safe_str(const char *s);

/*! \brief return true if a == b
 *
 * This is a safe variant, so it handles NULL inputs well
 * a == b == NULL ->true
 * */
bool str_eq(const char *a, const char *b);

//! Return true if string representation of average step is supported 
bool is_average_step_supported (const char *step);

//! Return true if string representation of average type is supported 
bool is_average_type_supported (const char *type);

//! Supported average step expressed in seconds OR -1
int64_t average_step_seconds (const char *step);

//! Return timestamp of left margin of an interval (of sampled data) required to compute the first average of given 'step' since 'start_timestamp'
int64_t average_extend_left_margin (int64_t start_timestamp, const char *step);

//! Return timestamp of first possible computable average of given step since 'timestamp'
int64_t average_first_since (int64_t timestamp, const char *step);

/*!
 \brief transform rfc-11 defined datetime format to calendar time (unix timestamp).

  For $BIOS project rest api rfc-11 defines one common format for datetimes. It is "derived" from iso8601 in the following sense:
  all hyphen (-) colon (:) characters are removed and 'Z' character is always appended to the end that represents UTC timezone.
  Since the standard understands the two following dates to be equal "19991231235959Z" == "20000101000000Z" we can not use
  strptime() function as it fails on hh:mm:ss 24:00:00. Another needed workaround is the fact that mktime() uses local time zone.

 \return unix timestamp or -1 on failure
*/
int64_t datetime_to_calendar (const char *datetime);

//! portable version of timegm() taken from manual pages 
int64_t my_timegm (struct tm *tm);

// TODO: move string_to_X functioncs into utils.[hc] as soon as streq conflict is solved

/**
 * \brief converts char* to int64_t
 * \param value string (char *), containing the number
 * \return text value converted to int32_t
 *
 * Function converts text representation of number to int32_t
 * In case of error, INT32_MAX is returned and errno is set.
 *
 * The input value must be whole used. Strings like "42aaa" is
 * not converted to 42 but error is produced.
 */
int64_t string_to_int64( const char *value );

/**
 * \brief converts char* to int32_t
 * \param value string (char *), containing the number
 * \return text value converted to int32_t
 * \see string_to_int64
 *
 * In case of error INT32_MAX is returned and errno is set.
 */
int32_t string_to_int32( const char *value );

/**
 * \brief converts char* to uint64_t
 * \param value string (char *), containing the number
 * \return text value converted to uint64_t
 * \see string_to_int64
 *
 * In case of error UINT64_MAX is returned and errno is set.
 */
uint64_t string_to_uint64( const char *value );

/**
 * \brief converts char* to uint32_t
 * \param value string (char *), containing the number
 * \return text value converted to uint32_t
 * \see string_to_int64
 *
 * In case of error UINT32_MAX is returned and errno is set.
 */
uint32_t string_to_uint32( const char *value );

int16_t string_to_int16( const char *value );
uint16_t string_to_uint16( const char *value );

int8_t string_to_int8( const char *value );
uint8_t string_to_uint8( const char *value );


// Macros
#define STR(X) #X

#define FREE0(X) \
    if (X) { \
        free (X); \
        X = NULL; \
    }

#ifdef __cplusplus
}
#endif
