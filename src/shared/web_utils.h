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

std::string
    json_key_value_s(
        const std::string &key,
        const std::string &value);

std::string
   json_key_value_u(
        const std::string &key,
        const uint64_t    &value);

std::string
   json_key_value_i(
        const std::string &key,
        const int64_t     &value);

std::string
   json_key_value_d(
        const std::string &key,
        const double      &value);

std::string
    create_error_json(
        const std::string &msg,
        int                code);

#endif // SRC_SHARED_WEB_UTILS_H_
