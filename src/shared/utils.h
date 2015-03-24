/* utils.h: various random helpers for C
 
Copyright (C) 2014 Eaton
 
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
 
Description: various random helpers for C
*/

#pragma once

#include <stdbool.h>
#include <string.h>

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
bool streq(const char *a, const char *b);

//! Return true if string representation of average step is supported 
bool is_average_step_supported (const char *step);

#ifdef __cplusplus
}
#endif
