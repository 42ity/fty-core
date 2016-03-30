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
    process_powers_key(
        const cxxtools::SerializationInfo &powers_si,
        std::vector <std::vector<cxxtools::String> > &data
    )
{
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
}


static void
    process_groups_key(
        const cxxtools::SerializationInfo &groups_si,
        std::vector <std::vector<cxxtools::String> > &data
    )
{
    if ( groups_si.category () != cxxtools::SerializationInfo::Array ) {
        throw std::invalid_argument("Key 'groups' should be an array");
    }
    // we need a counter for fields
    int i = 1;
    for ( const auto &oneElement : groups_si ) { // iterate through the array
        // id is just an informational field, ignore it here
        // name is mandatory
        if (  oneElement.findMember("name") == NULL ) {
            throw std::invalid_argument("Key 'name' in the key 'groups' is mandatory");
        }
        else {
            cxxtools::String group_name{};
            oneElement.getMember("name") >>= group_name;
            data[0].push_back (cxxtools::String ("group." + std::to_string(i)));
            data[1].push_back (group_name);
        }
        i++;
    }
}


static void
    process_ips_key(
        const cxxtools::SerializationInfo &si,
        std::vector <std::vector<cxxtools::String> > &data
    )
{
    LOG_START;
    if ( si.category () != cxxtools::SerializationInfo::Array ) {
        throw std::invalid_argument("Key 'ips' should be an array");
    }
    // we need a counter for fields
    int i = 1;
    for ( const auto &oneElement : si ) { // iterate through the array
        std::string value;
        oneElement.getValue(value);
        data[0].push_back (cxxtools::String ("ip." + std::to_string(i)));
        data[1].push_back (cxxtools::String (value));
        i++;
    }
}


static void
    process_macs_key(
        const cxxtools::SerializationInfo &si,
        std::vector <std::vector<cxxtools::String> > &data
    )
{
    LOG_START;
    if ( si.category () != cxxtools::SerializationInfo::Array ) {
        throw std::invalid_argument("Key 'macs' should be an array");
    }
    // we need a counter for fields
    int i = 1;
    for ( const auto &oneElement : si ) { // iterate through the array
        std::string value;
        oneElement.getValue(value);
        data[0].push_back (cxxtools::String ("mac." + std::to_string(i)));
        data[1].push_back (cxxtools::String (value));
        i++;
    }
}


static void
    process_hostnames_key(
        const cxxtools::SerializationInfo &si,
        std::vector <std::vector<cxxtools::String> > &data
    )
{
    LOG_START;
    if ( si.category () != cxxtools::SerializationInfo::Array ) {
        throw std::invalid_argument("Key 'hostnames' should be an array");
    }
    // we need a counter for fields
    int i = 1;
    for ( const auto &oneElement : si ) { // iterate through the array
        std::string value;
        oneElement.getValue(value);
        data[0].push_back (cxxtools::String ("hostname." + std::to_string(i)));
        data[1].push_back (cxxtools::String (value));
        i++;
    }
}


static void
    process_fqdns_key(
        const cxxtools::SerializationInfo &si,
        std::vector <std::vector<cxxtools::String> > &data
    )
{
    LOG_START;
    if ( si.category () != cxxtools::SerializationInfo::Array ) {
        throw std::invalid_argument("Key 'fqdns' should be an array");
    }
    // we need a counter for fields
    int i = 1;
    for ( const auto &oneElement : si ) { // iterate through the array
        std::string value;
        oneElement.getValue(value);
        data[0].push_back (cxxtools::String ("fqdn." + std::to_string(i)));
        data[1].push_back (cxxtools::String (value));
        i++;
    }
}

static void
    process_oneOutlet(
        const cxxtools::SerializationInfo &outlet_si,
        std::vector <std::vector<cxxtools::String> > &data
    )
{
    if ( outlet_si.category () != cxxtools::SerializationInfo::Array ) {
        throw std::invalid_argument("Key '" + outlet_si.name() +"' should be an array");
    }
    std::string name;
    std::string value;
    bool isReadOnly = false;
    for ( const auto &oneOutletAttr : outlet_si ) { // iterate through the outletAttributes
        try {
            oneOutletAttr.getMember("name").getValue(name);
            oneOutletAttr.getMember("value").getValue(value);
            oneOutletAttr.getMember("read_only").getValue(isReadOnly);
        }
        catch (...) {
            throw std::invalid_argument("In outlet object key 'name/value/read_only' is missing");
        }
        data[0].push_back(cxxtools::String ("outlet." + outlet_si.name() + "." + name));
        data[1].push_back(cxxtools::String (value));
    }
}


static void
    process_outlets_key(
        const cxxtools::SerializationInfo &outlets_si,
        std::vector <std::vector<cxxtools::String> > &data
    )
{
    if ( outlets_si.category () != cxxtools::SerializationInfo::Object ) {
        throw std::invalid_argument("Key 'outlets' should be an object");
    }
    for ( const auto &oneOutlet : outlets_si ) { // iterate through the object
        process_oneOutlet (oneOutlet, data);
    }
}



// forward declaration
static void
s_read_si(
        const cxxtools::SerializationInfo& si,
        std::vector <std::vector<cxxtools::String> >& data);

static void
    process_ext_key(
        const cxxtools::SerializationInfo &ext_si,
        std::vector <std::vector<cxxtools::String> > &data
    )
{
    if ( ext_si.category () == cxxtools::SerializationInfo::Array ) {
        log_debug ("it is GET format");
        // this information in GET format
        for ( const auto &oneAttrEl : ext_si ) { // iterate through the array
            // ASSUMPTION: oneAttr has 2 fields:
            // "read_only" - this is an information only
            // "some_unknown_name"
            for ( unsigned int i = 0 ; i < 2 ; ++i) {
                auto &oneAttr = oneAttrEl.getMember(i);
                auto &name = oneAttr.name();
                if ( name == "read_only" ) {
                    continue;
                }
                std::string value;
                oneAttr.getValue(value);
                data[0].push_back(cxxtools::String(name));
                data[1].push_back(cxxtools::String(value));
            }
        }
    }
    else if ( ext_si.category () == cxxtools::SerializationInfo::Object ) {
        // this information in PUT POST format
        log_debug ("it is PUT/POST format");
        s_read_si(ext_si, data);
    }
    else {
        throw std::invalid_argument("Key 'ext' should be an Array or Object");
    }
}

static void
s_read_si(
        const cxxtools::SerializationInfo& si,
        std::vector <std::vector<cxxtools::String> >& data)
{
    if ( data.size() != 2 ) {
        throw std::invalid_argument("Expected two items in array, got " + std::to_string(data.size()));
    }

    for (auto it = si.begin();
                    it != si.end(); ++it) {
        const cxxtools::String name = cxxtools::convert<cxxtools::String>(it->name());
        if ( name == "ext" ) {
            process_ext_key (si.getMember("ext"), data);
            continue;
        }
        // these fields are just for the information in the REPRESENTATION
        // may be we can remove them earlier, but for now it is HERE
        // TODO: BIOS-1428
        if ( name == "location_id" ) {
            continue;
        }
        if ( name == "location_uri" ) {
            continue;
        }

        if ( name == "powers" ) {
            process_powers_key (si.getMember("powers"), data);
            continue;
        }
        if ( name == "groups" ) {
            process_groups_key (si.getMember("groups"), data);
            continue;
        }
        if ( name == "outlets" ) {
            process_outlets_key (si.getMember("outlets"), data);
            continue;
        }
        if ( name == "ips" ) {
            process_ips_key (si.getMember("ips"), data);
            continue;
        }
        if ( name == "macs" ) {
            process_macs_key (si.getMember("macs"), data);
            continue;
        }
        if ( name == "hostnames" ) {
            process_hostnames_key (si.getMember("hostnames"), data);
            continue;
        }
        if ( name == "fqdns" ) {
            process_fqdns_key (si.getMember("fqdns"), data);
            continue;
        }
        std::string value;
        it->getValue(value);
        data[0].push_back(name);
        data[1].push_back(cxxtools::String(value));
    }
}

CsvMap
CsvMap_from_serialization_info(
        const cxxtools::SerializationInfo& si)
{
    std::vector <std::vector<cxxtools::String> > data = {{}, {}};
    s_read_si(si, data);
    // print the data
    //for ( unsigned int i = 0; i < data.size(); i++ ) {
    //    log_debug ("%s = %s", (cxxtools::convert<std::string> (data.at(0).at(i)) ).c_str(), (cxxtools::convert<std::string> (data.at(1).at(i))).c_str());
    // }
    CsvMap cm{data};
    cm.deserialize();
    return cm;
}


} //namespace shared
