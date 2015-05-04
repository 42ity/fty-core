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
Author(s): Tomas Halman <TomasHalman@eaton.com>
 
Description: TODO
*/

#ifndef SRC_SHARED_UTILS_APP_H__
#define SRC_SHARED_UTILS_APP_H__

#include "app.h"

#ifdef __cplusplus
extern "C"
{
#endif


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

// TODO: Tomas Halman should write documentation
int32_t
app_params_first_int32(app_t* msg);

int64_t
app_params_first_int64(app_t* msg);

uint32_t
app_params_first_uint32(app_t* msg);

uint64_t
app_params_first_uint64(app_t* msg);

int32_t
app_params_next_int32(app_t* msg);

int64_t
app_params_next_int64(app_t* msg);

int32_t
app_params_next_int32(app_t* msg);

int64_t
app_params_next_int64(app_t* msg);

void
app_params_append_int32(app_t* msg, int32_t value );

void
app_params_append_int64(app_t* msg, int64_t value );

void
app_args_set_string(app_t* msg, const char *key, const char *value);

void
app_args_set_int64(app_t* msg, const char *key, int64_t value );

#define app_args_set_int32 app_args_set_int64

void
app_args_set_uint64(app_t* msg, const char *key, uint64_t value );

#define app_args_set_uint32 app_args_set_uint64

int32_t
app_args_int32(app_t* msg, const char *key);

int64_t
app_args_int64(app_t* msg, const char *key);

uint32_t
app_args_uint32(app_t* msg, const char *key);

uint64_t
app_args_uint64(app_t* msg, const char *key);

#ifdef __cplusplus
}
#endif

#endif // SRC_SHARED_UTILS_APP_H__  
