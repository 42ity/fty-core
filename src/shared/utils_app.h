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

/*! \file   utils_app.h
    \brief  Not yet documented file
    \author Tomas Halman <TomasHalman@Eaton.com>
*/

#ifndef SRC_SHARED_UTILS_APP_H__
#define SRC_SHARED_UTILS_APP_H__

#include "app.h"

#ifdef __cplusplus
extern "C"
{
#endif

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
#define app_args_set_int16 app_args_set_int64
#define app_args_set_int8 app_args_set_int64

void
app_args_set_uint64(app_t* msg, const char *key, uint64_t value );

#define app_args_set_uint32 app_args_set_uint64
#define app_args_set_uint16 app_args_set_uint64
#define app_args_set_uint8 app_args_set_uint64

int8_t
app_args_int8(app_t* msg, const char *key);

int16_t
app_args_int16(app_t* msg, const char *key);

int32_t
app_args_int32(app_t* msg, const char *key);

int64_t
app_args_int64(app_t* msg, const char *key);

uint8_t
app_args_uint8(app_t* msg, const char *key);

uint16_t
app_args_uint16(app_t* msg, const char *key);

uint32_t
app_args_uint32(app_t* msg, const char *key);

uint64_t
app_args_uint64(app_t* msg, const char *key);

#ifdef __cplusplus
}
#endif

#endif // SRC_SHARED_UTILS_APP_H__  
