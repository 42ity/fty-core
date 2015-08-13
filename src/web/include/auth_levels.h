/* 
Copyright (C) 2014 Eaton
 
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

/*! \file   auth_levels.h
    \brief  TODO
    \author Jim Klimov <EvgenyKlimov@eaton.com>
*/

#ifndef SRC_WEB_INCLUDE_AUTH_LEVELS_H
#define SRC_WEB_INCLUDE_AUTH_LEVELS_H

//extern int8_t access_auth_level;
/* // Consumers require this block in ECPP templates:
 <%request scope="global">
 int8_t access_auth_level;
 </%request>
   // Processing as a std::string may require casting into (int) first
 */
    
/* Define known auth levels */
#define AUTH_LEVEL__MIN             -2
#define AUTH_LEVEL_ERROR_INVALID    -2  // Token did not pass checks
#define AUTH_LEVEL_ERROR_EMPTY      -1  // Token was passed but empty
/* Level <= error == no valid session exists, token not accepted */
#define AUTH_LEVEL__ERROR           -1
#define AUTH_LEVEL_ANONYMOUS        0   // No token was passed
/* Level >= authorized == valid session exists */
#define AUTH_LEVEL__AUTHORIZED      1
#define AUTH_LEVEL_USER             1   // Identified as a minimal user
#define AUTH_LEVEL_POWERUSER        2   // Identified as a "power-user"
#define AUTH_LEVEL_ADMIN            3   // Identified as an administrator
#define AUTH_LEVEL__MAX             3

#endif // SRC_WEB_INCLUDE_AUTH_LEVELS_H
