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
#include "assetcrud.h"
#include "dbpath.h"

namespace shared {

/* Workaround for a fact a) std::transform to do a strip and lower is weird, b) it breaks the map somehow*/
static const std::string _ci_strip(const std::string& str) {
     std::ostringstream b;

     for (const char c: str) {
         // allowed chars [a-zA-Z0-9_\.]
         if (::isalnum(c) || c == '_' || c == '.')
            b << static_cast<char>(::tolower(c));
     }

     return b.str();
}

CsvMap::CsvMap(const CsvMap::CxxData& data) :
    _data{},
    _title_to_index{}
{
    //XXX: this is ugly part, which takes the table of cxxtools::String and convert it to table of utf-8 encoded std::string
    for (size_t i = 0; i != data.size(); i++) {
        auto row = std::vector<std::string>{};
        row.reserve(data[0].size());

        /*FIXME: segfaults!
         * std::transform(data[i].begin(), data[i].end(), row.begin(),
                [&](const cxxtools::String& s) -> std::string { return to_utf8(s); });
        */
        for (const auto& s: data[i]) {
            row.push_back(cxxtools::Utf8Codec::encode(s));
        }
        _data.push_back(row);
    }
}

void CsvMap::deserialize() {

    size_t i = 0;
    for (const std::string& title_name : _data[0]) {

        std::string title = _ci_strip(title_name);

        if (_title_to_index.count(title) == 1) {
            std::ostringstream buf;
            buf << "duplicate title name '" << title << "'";
            throw std::invalid_argument(buf.str());
        }

        _title_to_index.emplace(title, i);
        i++;
    }
}

const std::string& CsvMap::get(size_t row_i, const std::string& title_name) const {


    if (row_i >= _data.size()) {
        std::ostringstream buf;
        buf << "row_index " << row_i << " was out of range " << _data.size();
        throw std::out_of_range(buf.str());
    }

    std::string title = _ci_strip(title_name);

    if (_title_to_index.count(title) == 0) {
        std::ostringstream buf;
        buf << "title name '" << title << "' not found";
        throw std::out_of_range{buf.str()};
    }

    size_t col_i = _title_to_index.at(title);
    return _data[row_i][col_i];
}

std::string CsvMap::get_strip(size_t row_i, const std::string& title_name) {
    return _ci_strip(get(row_i, title_name));
}

bool CsvMap::hasTitle(const std::string& title_name) const {
    std::string title = _ci_strip(title_name);
    return (_title_to_index.count(title) == 1);
}

std::set<std::string> CsvMap::getTitles() const {
    std::set<std::string> ret{};
    for (auto i : _title_to_index) {
        ret.emplace(i.first);
    }
    return ret;
}

//TODO: does not belongs to csv, move somewhere else
void skip_utf8_BOM (std::istream& i) {
    int c1, c2, c3;
    c1 = i.get();
    c2 = i.get();
    c3 = i.get();

    if (c1 == 0xef && c2 == 0xbb && c3 == 0xbf)
        return;

    i.putback(c3);
    i.putback(c2);
    i.putback(c1);
}

} //namespace shared
