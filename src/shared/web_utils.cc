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
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 */

#include "web_utils.h"

std::string
    json_key_value_s(
        const std::string &key,
        const std::string &value)
{
    // make a string like this
    // "key":"value"
    std::string s = "\"" + key;
    s += "\":\"";
    s += value;
    s += "\"";
    return s;
};

std::string
   json_key_value_u(
        const std::string &key,
        const uint64_t &value)
{
    // make a string like this
    // "key":value
    std::string s = "\"" + key;
    s += "\":";
    s += std::to_string(value);
    return s;
};

std::string
   json_key_value_i(
        const std::string &key,
        const int64_t &value)
{
    // make a string like this
    // "key":value
    std::string s = "\"" + key;
    s += "\":";
    s += std::to_string(value);
    return s;
};

std::string
   json_key_value_d(
        const std::string &key,
        const double &value)
{
    // make a string like this
    // "key":value
    std::string s = "\"" + key;
    s += "\":";
    s += std::to_string(value);
    return s;
};

std::string
    create_error_json(
        const std::string &msg,
        int                code)
{
    std::string s = "{\n\t\"errors\": [\n\t  {\n\t\t\"message\" : \"";
    s+=msg;
    s+="\",\n\t\t\"code\" : ";
    s+=std::to_string(code);
    s+="\n\t  }\n\t]\n}";
    return s;
}
