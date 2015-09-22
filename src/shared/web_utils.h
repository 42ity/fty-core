/*
 *
 * Copyright (C) 2015 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*!
 * \file   web_utils.h
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \brief  some helpers for JSON representation
 */
#ifndef SRC_SHARED_WEB_UTILS_H_
#define SRC_SHARED_WEB_UTILS_H_

#include <string>

/*
 * \brief Creates a string in the form "key":"value"
 *        without any {,} (just pair key value). As
 *        value is string it should be in quotes.
 *
 * \param key   - a key
 * \param value - a string value
 *
 * \return "key":"value"
 */
std::string
    json_key_value_s(
        const std::string &key,
        const std::string &value);

/*
 * \brief Creates a string in the form "key":value
 *        without any {,} (just pair key value). As
 *        value is not a string it should not be in quotes.
 *
 * \param key   - a key
 * \param value - an unsigned value
 *
 * \return "key":value
 */
std::string
   json_key_value_u(
        const std::string &key,
        const uint64_t    &value);

/*
 * \brief Creates a string in the form "key":value
 *        without any {,} (just pair key value). As
 *        value is not a string it should not be in quotes.
 *
 * \param key   - a key
 * \param value - an integer value
 *
 * \return "key":value
 */
std::string
   json_key_value_i(
        const std::string &key,
        const int64_t     &value);

/*
 * \brief Creates a string in the form "key":value
 *        without any {,} (just pair key value). As
 *        value is not a string it should not be in quotes.
 *
 * Pricisios: 6 numbers after point always (even zeros)
 *
 * \param key   - a key
 * \param value - a double value
 *
 * \return "key":value
 */
std::string
   json_key_value_d(
        const std::string &key,
        const double      &value);

/*
 * \brief Creates a string that containes JSON error message
 *        with only one error.
 *
 *
 *  Returned string has the following structure:
 *
 *  {
 *      errors: [
 *          {
 *              "message": "@msg",
 *              "code": @code
 *          }
 *      ]
 *  }
 *
 * \param msg   - a message that should be displayed to the user
 * \param code  - a code for an error
 *
 * \return JSON string
 */
std::string
    create_error_json(
        const std::string &msg,
        int                code);

#endif // SRC_SHARED_WEB_UTILS_H_
