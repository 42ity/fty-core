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

/*!
 \file utils++.h
 \brief c++ utilities 
 \author Karol Hrdina <karolhrdina@eaton.com>
*/

#ifndef SRC_SHARED_UTILS_PLUSPLUS_H__
#define SRC_SHARED_UTILS_PLUSPLUS_H__

#include <string>

namespace utils {

namespace math {

/*!
 \brief Format double \a number into std::string \a result with floating point precision given by \a precision 
 \todo Decide if noexcept (not throwing, return int) or follow a style of letting the exception bubble up
*/
void dtos (double number, std::streamsize precision, std::string& result);

} // namespace utils::math

} // namespace utils

#endif // SRC_SHARED_UTILS_PLUSPLUS_H__



