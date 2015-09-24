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

/*! \file utils++.h
    \brief c++ utilities 
    \author Karol Hrdina <KarolHrdina@Eaton.com>
*/

#ifndef SRC_SHARED_UTILS_PLUSPLUS_H__
#define SRC_SHARED_UTILS_PLUSPLUS_H__

#include <string>
#include <map>
#include <czmq.h>

namespace utils {

namespace math {

/*!
 \brief Format double \a number into std::string \a result with floating point precision given by \a precision 
 \todo Decide if noexcept (not throwing, return int) or follow a style of letting the exception bubble up
*/
void dtos (double number, std::streamsize precision, std::string& result);

} // namespace utils::math

namespace json {

/*!
 \brief Escape string for json output
 \return Escaped json on success, "(null_ptr)" string on null argument
*/
std::string escape (const char *string);

/*!
 \brief Convenient wrapper for escape"("const char *string")"
*/
std::string escape (const std::string& before);

template <typename T
        , typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
std::string make (T t) {
    try {
        return escape (std::to_string (t));
    } catch (...) {
        return "";
    }
}

// TODO: doxy
//  basically, these are property "makers"; you supply any json-ifiable type pair and it creates a valid, properly escaped property (key:value) pair out of it.
// single arg version escapes and quotes were necessary (i.e. except int types...)

template <typename T
        , typename = typename std::enable_if<std::is_convertible<T, std::string>::value>::type>
std::string make (const T& t) {
    try {
        return std::string ("\"").append (escape (t)).append ("\"");
    } catch (...) {
        return "";
    }
}

template <typename S
        , typename = typename std::enable_if<std::is_convertible<S, std::string>::value>::type
        , typename T
        , typename = typename std::enable_if<std::is_convertible<T, std::string>::value>::type>
std::string make (const S& key, const T& value) {
    return std::string (make (key)).append (" : ").append (make (value));
}

template <typename S
        , typename = typename std::enable_if<std::is_convertible<S, std::string>::value>::type
        , typename T
        , typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
std::string make (const S& key, T value) {
    return std::string (make (key)).append (" : ").append (make (value));
}

template <typename S
        , typename = typename std::enable_if<std::is_convertible<S, std::string>::value>::type
        , typename T
        , typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
std::string make (T key, const S& value) {
    return std::string ("\"").append (make (key)).append ("\" : ").append (make (value));
}

template <typename T
        , typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
std::string make (T key, T value) {
    return std::string ("\"").append (make (key)).append ("\" : ").append (make (value));
}

} // namespace utils::json

/*!
 * \brief universal escaping function
 *
 * \param[in] in is input string to be escaped
 * \param[in] escape_chars is list of characters to be escaped
 *
 * \todo TODO: escaping of backslash itself is not implemented and if present in escape_chars, nothing is escaped
 *
 * \return escaped string
 */
std::string
escape (
        const std::string& in,
        const std::string& escape_chars);

/*!
 * \brief escape special characters for SQL (_ and %)
 *
 * \param[in] in is input string to be escaped
 *
 * \return escaped string
 */
std::string
sql_escape(
        const std::string& in);

/*
 * \brief convert zhash to map<string,string>
 *
 * \param zhash to convert
 * \return std::map
 */
std::map<std::string,std::string>
zhash_to_map(zhash_t *hash);

} // namespace utils

#endif // SRC_SHARED_UTILS_PLUSPLUS_H__



