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
#include <cstring>
#include <ostream>
#include <limits>
#include <cxxtools/jsonformatter.h>
#include <cxxtools/convert.h>
#include <cxxtools/regex.h>

#include "utils_web.h"

namespace utils {

uint32_t
string_to_element_id (const std::string& string) {

    size_t pos = 0;
    unsigned long long u = std::stoull (string, &pos);
    if (pos != string.size ()) {
        throw std::invalid_argument ("string_to_element_id");
    }
    if (u == 0 || u >  std::numeric_limits<uint32_t>::max()) {
        throw std::out_of_range ("string_to_element_id");
    }
    uint32_t element_id = static_cast<uint32_t> (u);
    return element_id;
}

namespace json {    

std::string escape (const char *string) {
    if (!string)
        return "(null_ptr)";

    std::string after;
    std::string::size_type length = strlen (string);
    after.reserve (length * 2);

/*
    Quote from http://www.json.org/
    -------------------------------
    Char
    any-Unicode-character-except-"-or-\-or-control-character:
        \"
        \\
        \/
        \b
        \f
        \n
        \r
        \t
        \u four-hex-digits 
    ------------------------------
*/

    for (std::string::size_type i = 0; i < length; ++i) {
        char c = string[i];
        if (c == '"') {
            after.append ("\\\"");
        }
        else if (c =='\b') {
            after.append ("\\\\b");
        }
        else if (c =='\f') {
            after.append ("\\\\f");
        }
        else if (c == '\n') {
            after.append ("\\\\n");
        }
        else if (c == '\r') {
            after.append ("\\\\r");
        }
        else if (c == '\t') {
            after.append ("\\\\t");
        }
        else if (c == '\\') {
            after.append ("\\\\\\");
        }
        else if (static_cast<unsigned char>(c) >= 0x80 || static_cast<unsigned char>(c) < 0x20) {
            // Code below this comment is taken from
            //      https://github.com/maekitalo/cxxtools
            //      JsonFormatter::stringOut(const std::string& str) {}
            //      Author: Tommi Mäkitalo (tommi@tntnet.org)
            after.append("\\u");
            static const char hex[] = "0123456789abcdef";
            uint32_t v = static_cast<unsigned char>(c);

            for (uint32_t s = 16; s > 0; s -= 4)
            {
                after += hex[(v >> (s - 4)) & 0xf];
            }
        }
        else {
            after += c;
        }
    }
    return after;
}

std::string escape (const std::string& before) {
    return escape (before.c_str ());
}


// Note: At the time of re-writing from defect cxxtools::JsonFormatter I decided
// to go with the simplest solution for create_error_json functions (after all,
// our error template is not that complex). Shall some person find this ugly
// and repulsive enough, there is a working cxxtools::JsonSerialize solution
// ready at the following link:
// http://stash.mbt.lab.etn.com/projects/BIOS/repos/core/pull-requests/1094/diff#src/web/src/error.cc

std::string
create_error_json (const std::string& message, uint32_t code) {
    return std::string (
"{\n"
"\t\"errors\": [\n"
"\t\t{\n"
"\t\t\t\"message\": ").append (jsonify (message)).append (",\n"
"\t\t\t\"code\": ").append (jsonify (code)).append ("\n"
"\t\t}\n"
"\t]\n"
"}\n"
);
}

std::string
create_error_json (const std::string& message, uint32_t code, const std::string& debug) {
    return std::string (
"{\n"
"\t\"errors\": [\n"
"\t\t{\n"
"\t\t\t\"message\": ").append (jsonify (message)).append (",\n"
"\t\t\t\"debug\": ").append (jsonify (debug)).append (",\n"
"\t\t\t\"code\": ").append (jsonify (code)).append ("\n"
"\t\t}\n"
"\t]\n"
"}\n"
);
}

std::string
create_error_json (std::vector <std::tuple<uint32_t, std::string, std::string>> messages) {
    std::string result =
"{\n"
"\t\"errors\": [\n";

    for (const auto &it : messages) {
        result.append (
"\t\t{\n"
"\t\t\t\"message\": "
        );
        result.append (jsonify (std::get<1>(it))).append (
",\n");
        if (std::get<2>(it) != "") {
        result.append (
"\t\t\t\"debug\": "
        );
        result.append (jsonify (std::get<2>(it))).append (
",\n");
        }
        result.append (
"\t\t\t\"code\": "
        );
        result.append (jsonify (std::get<0>(it))).append (
"\n"
"\t\t},\n"
        );
    }
    // remove ,\n
    result.pop_back ();
    result.pop_back ();

    result.append (
"\n\t]\n"
"}\n"
);
    return result;
}

std::string jsonify (double t)
{
    if (isnan(t))
        return "null";
    return std::to_string (t);
}

} // namespace utils::json
} // namespace utils

