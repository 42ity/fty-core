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

#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <set>
#include <sstream>

#include <cxxtools/csvdeserializer.h>

#include "csv.h"
#include "assetcrud.h"
#include "dbpath.h"
#include "log.h"

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

    if (_data.size() == 0) {
        throw std::invalid_argument("Can't process empty data set");
    }

    size_t i = 0;
    for (const std::string& title_name : _data[0]) {
        // TODO hotfix ! remove later!!!
        if ( title_name.empty() )
            continue;

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
    if (col_i >= _data[row_i].size()) {
        throw std::out_of_range{
            "On line " + std::to_string(row_i+1) + \
            ": requested column " + title_name + " (index " + std::to_string(col_i +1) + \
            ") where maximum is " + std::to_string(_data[row_i].size())};
    }
    return _data[row_i][col_i];
}

std::string CsvMap::get_strip(size_t row_i, const std::string& title_name) const{
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

char
findDelimiter(
        std::istream& i,
        std::size_t max_pos)
{
    for (std::size_t pos = 0; i.good() && !i.eof() && pos != max_pos; pos++) {
        i.seekg(pos);
        char ret = i.peek();
        if (ret == ',' || ret == ';' || ret == '\t') {
            i.seekg(0);
            return ret;
        }
    }
    i.seekg(0);
    return '\x0';
}

bool
hasApostrof(
        std::istream& i)
{
    for (std::size_t pos = 0; i.good() && !i.eof(); pos++) {
        i.seekg(pos);
        char ret = i.peek();
        if (ret == '\'') {
            i.seekg(0);
            return true;
        }
    }
    i.seekg(0);
    return false;
}
CsvMap
CsvMap_from_istream(
        std::istream& in)
{
    std::vector <std::vector<cxxtools::String> > data;
    cxxtools::CsvDeserializer deserializer(in);
    if ( hasApostrof(in) ) {
        const char* msg = "CSV file contains ' (apostrof), please remove it";
        log_error("%s\n", msg);
        LOG_END;
        throw std::invalid_argument (msg);
    }
    char delimiter = findDelimiter(in);
    if (delimiter == '\x0') {
        const char* msg = "Cannot detect the delimiter, use comma (,) semicolon (;) or tabulator";
        log_error("%s\n", msg);
        LOG_END;
        throw std::invalid_argument(msg);
    }
    log_debug("Using delimiter '%c'", delimiter);
    deserializer.delimiter(delimiter);
    deserializer.readTitle(false);
    deserializer.deserialize(data);
    CsvMap cm{data};
    cm.deserialize();
    return cm;
}

static void
s_read_si(
        const cxxtools::SerializationInfo& si,
        std::vector <std::vector<cxxtools::String> >& data)
{
    if (data.size() != 2)
        throw std::invalid_argument("Expected two items in array, got " + std::to_string(data.size()));

    bool has_ext;

    for (auto it = si.begin();
                    it != si.end(); ++it) {
        const cxxtools::String name = cxxtools::convert<cxxtools::String>(it->name());

        if (name == "ext") {
            has_ext = true;
            continue;
        }

        if ( name == "powers" ) {
            auto powers_si = si.getMember("powers");
            if ( powers_si.category () != cxxtools::SerializationInfo::Array ) {
                throw std::invalid_argument("Key 'powers' should be an array");
            }
            // we need a counter for fields
            int i = 1;
            for ( const auto &oneElement : powers_si ) { // iterate through the array
                // src_name is mandatory
                if (  oneElement.findMember("src_name") == NULL ) {
                    throw std::invalid_argument("Key 'src_name' in the key 'powers' is mandatory");
                }

                cxxtools::String src_name{};
                oneElement.getMember("src_name") >>= src_name;
                data[0].push_back (cxxtools::String("power_source." + std::to_string(i)));
                data[1].push_back (src_name);
                // src_outlet is optimal
                if ( oneElement.findMember("src_socket") != NULL ) {
                    cxxtools::String src_socket{};
                    oneElement.getMember("src_socket") >>= src_socket;
                    data[0].push_back (cxxtools::String("power_plug_src." + std::to_string(i)));
                    data[1].push_back (src_socket);
                }
                // dest_outlet is optional
                if ( oneElement.findMember("dest_socket") != NULL ) {
                    cxxtools::String dest_socket{};
                    oneElement.getMember("dest_socket") >>= dest_socket;
                    data[0].push_back (cxxtools::String ("power_input." + std::to_string(i)));
                    data[1].push_back (dest_socket);
                }
                // src_id is there, but here it is ignored
                // because id and name can be in the conflict.
                // src_name is mutable, but src_id is just an informational field
                i++;
            }
            continue;
        }
        cxxtools::String value;
        it->getValue(value);
        data[0].push_back(name);
        data[1].push_back(value);
    }

    if (!has_ext)
        return;

    const cxxtools::SerializationInfo* ext_si_p = si.findMember("ext");
    if (!ext_si_p)
        return;
    s_read_si(*ext_si_p, data);
}

CsvMap
CsvMap_from_serialization_info(
        const cxxtools::SerializationInfo& si)
{
    std::vector <std::vector<cxxtools::String> > data = {{}, {}};
    s_read_si(si, data);
    CsvMap cm{data};
    cm.deserialize();
    return cm;
}

CsvMap
CsvMap_from_serialization_info(
        const cxxtools::SerializationInfo &si,
        const std::string                 &id)
{
    std::vector <std::vector<cxxtools::String> > data = {{}, {}};
    s_read_si(si, data);

    cxxtools::SerializationInfo si_id;
    si_id.setTypeName("id_si");
    si_id.addMember("id") <<= id;
    s_read_si(si_id, data);

    CsvMap cm{data};
    cm.deserialize();
    return cm;
}


} //namespace shared
