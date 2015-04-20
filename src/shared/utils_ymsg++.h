/* 
Copyright (C) 2015 Eaton
 
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
Author(s): Karol Hrdina <karolhrdina@eaton.com>
 
Description: ymsg_t helper functions for c++
*/

#ifndef SRC_SHARED_UTILS_YMSG_PLUSPLUS_H__
#define SRC_SHARED_UTILS_YMSG_PLUSPLUSH__

#include <string>
#include "ymsg.h"

//! Format the contents of the ROZP message into std::string for cases when it can't be printed on stdout using ymsg_print ().
void
    ymsg_format (ymsg_t *self, std::string& str);
    
#endif // SRC_SHARED_UTILS_YMSG_PLUSPLUS_H__    

