/* 
Copyright (C) 2015 Eaton
 
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

/*! \file   utils_ymsg++.h
    \brief  ymsg_t helper functions for c++
    \author Karol Hrdina <karolhrdina@eaton.com>
*/

#ifndef SRC_SHARED_UTILS_YMSG_PLUSPLUS_H__
#define SRC_SHARED_UTILS_YMSG_PLUSPLUS_H__

#include <string>
#include "ymsg.h"

//! Format the contents of the ROZP message into std::string for cases when it can't be printed on stdout using ymsg_print ().
void
    ymsg_format (ymsg_t *self, std::string& str);
    
#endif // SRC_SHARED_UTILS_YMSG_PLUSPLUS_H__    

