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

#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <set>
#include <sstream>

#include "csv.h"

namespace shared {

/* Workaround for a fact a) std::transform to do a strip and lower is weird, b) it breaks the map somehow*/
static const std::string _ci_strip(const std::string& str) {
     std::ostringstream b;

     for (const char c: str) {
         if (::isspace(c))
             continue;
         b << static_cast<char>(::tolower(c));
     }

     return b.str();
}

void CsvMap::deserialize() {

    size_t i = 0;
    for (const std::string& title_name : _data[0]) {

        std::string title = _ci_strip(title_name);

        if (_field_to_index.count(title) == 1) {
            std::ostringstream buf;
            buf << "duplicate field name '" << title << "'";
            throw std::invalid_argument(buf.str());
        }
        _field_to_index.emplace(title, i);
        i++;
    }
}

const std::string& CsvMap::get(size_t row_i, const std::string title_name) const {


    if (row_i >= _data.size()) {
        std::ostringstream buf;
        buf << "row_index " << row_i << " was out of range " << _data.size();
        throw std::out_of_range(buf.str());
    }

    std::string title = _ci_strip(title_name);

    if (_field_to_index.count(title) == 0) {
        std::ostringstream buf;
        buf << "field name '" << title << "' not found";
        throw std::out_of_range{buf.str()};
    }

    size_t col_i = _field_to_index.at(title);
    return _data[row_i][col_i];
}

} //namespace shared
