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
#include <sstream>

#include "utils++.h"

namespace utils {

namespace math {

void dtos (double number, std::streamsize precision, std::string& result) {
    std::ostringstream stream;
    stream.precision (precision);
    stream << std::fixed;

    stream << number;
    result.assign (stream.str ());
}

} // namespace utils::math

std::string escape (const std::string& in, const std::string& escape_chars) {

    std::stringstream s;

    if (in.empty() || escape_chars.empty())
        return in;

    if (in.size() == 1 && in.find_first_of(escape_chars) != std::string::npos)
        return "\\" + in;

    if (escape_chars.find('\\') != std::string::npos)
        return in;

    if (in.find_first_of(escape_chars) == std::string::npos) {
        return in;
    }

    size_t i = 0;
    while (i < in.size()) {
        if (in[i] == '\\') {
            s << in[i] << in[i+1];
            i++;
        }
        else if (in.substr(i, i+1).find_first_of(escape_chars) == 0) {
            s << '\\' << in[i];
        }
        else {
            s << in[i];
        }
        i++;
    }

    return s.str();
}

std::string sql_escape(const std::string& in) {
    return escape(in, "_%");
}

std::map<std::string,std::string> zhash_to_map(zhash_t *hash)
{
    std::map<std::string,std::string> map;
    char *item = (char *)zhash_first(hash);
    while(item) {
        const char * key = zhash_cursor(hash);
        const char * val = (const char *)zhash_lookup(hash,key);
        if( key && val ) map[key] = val;
        item = (char *)zhash_next(hash);
    }
    return map;
}

std::string
join (const char **str_arr, uint32_t length, const char *separator) {
    std::string result;
    if (!str_arr || !*str_arr || !separator)
        return result;
    result.append (str_arr[0]);
    for (uint32_t i = 1; i < length; ++i) {
        if (!str_arr[i])
            return result;
        result.append (separator).append (str_arr[i]);
    }
    return result;
}

std::string
join (const char **str_arr, const char *separator) {
    std::string result;
    if (!str_arr || !*str_arr || !separator) 
        return result;
    result.append (str_arr[0]);
    uint32_t i = 1;
    while (true) {
        if (!str_arr[i])
            return result;
        result.append (separator).append (str_arr[i]);
        ++i;
    }
}

} // namespace utils

