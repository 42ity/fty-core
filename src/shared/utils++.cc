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

void escape (const std::string& in, const std::string& escape_chars, std::string& out) {

    size_t pos =0;
    size_t prev_pos = 0;

    std::stringstream s;

    out.clear();
    if (in.empty() || escape_chars.empty()) {
        s << in;
        out = s.str();
        return;
    }

    if (in.size() == 1 && in.find_first_of(escape_chars) != std::string::npos) {
        out = '\\' + in;
        return;
    }

    if (escape_chars.find('\\') != std::string::npos) {
        return;
    }

    pos = in.find_first_of(escape_chars);
    if (pos == std::string::npos) {
        s << in;
        out = s.str();  //there must be better way how to copy string...
        return;
    }

    while (pos != std::string::npos)
    {
        if (in[pos-1] != '\\')
            s << in.substr(prev_pos, pos) << '\\' << in[pos];
        else
            s << in.substr(prev_pos, pos+1);
        prev_pos = pos;
        pos = in.find_first_of(escape_chars, pos+1);
    }
    if (prev_pos != pos)
        s << in.substr(prev_pos+1);

    out = s.str();
}

void sql_escape(const std::string& in, std::string& out) {
    return escape(in, "_%", out);
}

} // namespace utils

