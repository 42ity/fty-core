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

/*!
 * \brief universal escaping function
 *
 * \param[in] in is input string to be escaped
 * \param[in] escape_chars is list of characters to be escaped
 * \param[out] result is escaped string
 *
 * \todo TODO: escaping of backslash itself is not implemented and if present in escape_chars, out remains empty
 */
void
escape (
        const std::string& in,
        const std::string& escape_chars,
        std::string& out);

/*!
 * \brief escape special characters for SQL (_ and %)
 *
 * \param[in] in is input string to be escaped
 * \param[out] result is escaped string
 */
void
sql_escape(
        const std::string& in,
        std::string& out);

} // namespace utils

#endif // SRC_SHARED_UTILS_PLUSPLUS_H__



