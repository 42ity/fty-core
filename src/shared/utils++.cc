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

namespace json {

std::string escape (const char *string) {
    if (!string)
        return "(null_ptr)";

    std::string after;
    std::string::size_type length = strlen (string);
    after.reserve (length * 2);

    bool is_escaped = false;

    for (std::string::size_type i = 0; i < length; ++i) {
        char c = string[i];
        switch (c) {
            case '"':
                {
                    if (is_escaped) {
                        is_escaped = false;
                    }
                    after.append("\\\"");
                    break;
                }
            case '\\':
                {
                    if (is_escaped) {
                        after.append ("\\\\");
                        is_escaped = false;
                    }
                    else {
                        is_escaped = true;
                    }
                    break;
                }
            case 'b':
            case 'f':
            case 'n':
            case 'r':
            case 't':
            case 'u':
                {
                    if (is_escaped) {
                        after.append ("\\");
                        is_escaped = false;
                    }
                    after += c;
                    break;
                }
            default:
                {
                    if (is_escaped) {
                        after.append ("\\\\");
                        is_escaped = false;
                    }
                    after += c;
                }
        }
    }
    return after;
}

std::string escape (const std::string& before) {
    return escape (before.c_str ());
}

} // namespace utils::json

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

} // namespace utils

