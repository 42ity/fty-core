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

/*! \file   utils.h
    \brief  various random C and project wide helpers
    \author Michal Vyskocil <MichalVyskocil@Eaton.com>
    \author Karol Hrdina <KarolHrdina@Eaton.com>
*/

#ifndef SRC_SHARED_UTILS_H
#define SRC_SHARED_UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>

#include "agents.h" //definitions of common data types

// Macros
#define STR(X) #X

#define FREE0(X) \
    if (X) { \
        free (X); \
        X = NULL; \
    }

#define MAC_SIZEA   18  //size of array of chars for mac address including trailing \0

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

//! \brief convert unix time to rest api time
//
//  \param t - unix time
//  \param buf - caller's buffer large enough to store the string
//  \param s - size of the buffer
//  \return -1 in case of failure, 0 otherwise
int calendar_to_datetime (time_t timestamp, char* buffer, size_t n);

//! portable version of timegm() taken from manual pages 
int64_t my_timegm (struct tm *tm);

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

// inspired by https://gcc.gnu.org/onlinedocs/gcc/Integer-Overflow-Builtins.html#Integer-Overflow-Builtins
bool addi32_overflow(int32_t a, int32_t b, int32_t *res);

//! \brief convert alert_state_t to string
const char* alert_state_to_str(alert_state_t astate);

/**
 *  \brief rescale number to the new scale - upscalling loses information
 *
 *  \param in_value - input value to convert
 *  \param in_scale - input scale
 *  \param new_scale - new scale to be returned
 *  \param value - pointer to variable to store the result
 *
 *  \return true means success, false means overflow or underflow
 *
 */
bool bsi32_rescale(int32_t in_value, int8_t in_scale, int8_t new_scale, int32_t* value);

/** \brief binary scale add
 *
 *  \param value1 - first operand's value
 *  \param scale1 - first operand's scale
 *  \param value2 - second operand's value
 *  \param scale2 - second operand's scale
 *  \param value - pointer to variable to store the result value
 *  \param scale - pointer to variable to store the result scale
 *
 *  \return true means success, false means overflow or underflow
 */
bool bsi32_add(int32_t value1, int8_t scale1,
           int32_t value2, int8_t scale2,
           int32_t *value, int8_t* scale);

/** \brief get mac address for given name
 *
 * \param ethname name of interface
 * \param buf preallocated buffer (at minimum MAX_ADDR_LEN+1 long)
 * \param len length of buffer
 *
 * \return true if value has been read, or false if read failed or ethname was lo
 */
bool
get_mac(
        const char* ethname,
        char* buf,
        size_t len);
#ifdef __cplusplus
}
#endif

#endif // SRC_SHARED_UTILS_H
