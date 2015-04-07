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

std::vector<std::string> CsvMap::getTitles() const {
    std::vector<std::string> ret;
    std::copy(_data[0].cbegin(), _data[0].cend(),
            std::back_inserter(ret));
    return ret;
}

//TODO TODO TODO
// helper functions to check data from csv, move to an another location!!!

// check if it is valid location chain
// valid_location_chain("device", "rack") -> true
// valid_location_chain("room", "rack") -> false
static bool is_valid_location_chain(const std::string& type, const std::string& parent_type) {
    static const std::vector<std::string> LOCATION_CHAIN = \
        {"datacenter", "room", "row", "rack", "device"};

    size_t i, ti, pi;
    i = 0;
    for (auto x : LOCATION_CHAIN) {
        if (x == type)
            ti = i;
        if (x == parent_type)
            pi = i;
        i++;
    }
    return pi < ti;
}

// convert input '?[1-5]' to 1-5
static int get_priority(const std::string& s) {
    for (int i = 0; i != 2; i++) {
        if (s[i] <= 49 && s[i] >= 57) {
            return s[i] - 48;
        }
    }
    return 5;
}

static bool is_status(const std::string& status) {
    static const std::set<std::string> STATUSES = \
    {"active", "nonactive", "spare", "retired"};
    return (STATUSES.count(status));
}

static bool is_email(const std::string& str) {
    for (char ch : str) {
        if (ch == '@') {
            return true;
        }
    }
    return false;
}

static std::map<std::string,int> read_element_types(tntdb::Connection &conn)
{
    auto res = get_dictionary_element_type(conn);
    return res.item;
}

static std::map<std::string,int> read_device_types(tntdb::Connection &conn)
{
    auto res = get_dictionary_device_type(conn);
    return res.item;
}

static void process_row (tntdb::Connection &conn, CsvMap cm, size_t row_i)
{
    // TODO move somewhere else
    static const std::set<std::string> STATUSES = \
        {"active", "inactive"}; // TODO check
    static auto TYPES = read_element_types (conn);
    static auto SUBTYPES = read_device_types (conn);
    //       {"server", "sts", "ups", "epdu", "pdu", "genset", "main"};


    auto name = cm.get(row_i, "name");
    if ( !name.empty() )
        throw std::invalid_argument("name is empty");

    auto type = cm.get_strip(row_i, "type");
    if ( TYPES.find(type) == TYPES.end() )
        throw std::invalid_argument("Type '" + type + "' is not allowed");
    auto type_id = TYPES.find(type)->second;

    auto status = cm.get_strip(row_i, "status");
    if ( STATUSES.find(status) == STATUSES.end() )
    {
        throw std::invalid_argument( "Status '" + status + "' is not allowed");
        // OR
        // status = "nonactive";
    }

    auto bs_critical = cm.get_strip(row_i, "business_critical");
    if (bs_critical != "yes" && bs_critical != "no") {
        throw std::invalid_argument( "Business critical '" + status 
                            + "' is not allowed");
        // OR
        // bs_critical = "no";
    }
    // TODO function
    int bc = 1;
    if (bs_critical == "no")
        bc = 0;

    int priority = get_priority(cm.get_strip(row_i, "priority"));

    auto location = cm.get(row_i, "location");
    a_elmnt_id_t parent_id = 0;
    if ( !location.empty() )
    {
        auto ret = select_asset_element_by_name(conn, location.c_str());
        if ( ret.status == 1 )
            parent_id = ret.item.id;
        else
        {
            // jestli parent podle jmena se nenasel
            // throw ?????
        }
    }
    
    auto subtype = cm.get_strip(row_i, "subtype");
    if ( ( type == "device" ) && ( SUBTYPES.find(subtype) == SUBTYPES.end() ) ) {
        throw std::invalid_argument ("Subtype '" + subtype + "' is not allowed");
    }
    if ( ( type != "device" ) && ( type != "group") )
    {
        // need to write somewhere, that this column would be ignored
    }
    auto subtype_id = TYPES.find(type)->second;

    
    std::set <a_elmnt_id_t>  groups{}; // list of element id of all groups, the element belongs to   
    for (int group_index = 1 ; true; group_index++ )
    {
        try {
            auto group = cm.get(row_i, "group." + std::to_string(group_index));
            auto ret = select_asset_element_by_name(conn, group.c_str());
            if ( ret.status == 1 )
                groups.insert(ret.item.id);
        }
        catch (...)
        {
            break;
        }
    }
    
    std::set <link_t>  links{};
    // TODO read and fill this
    zhash_t           *extattributes = NULL;
    // all columns, that were not jet read, insert as ext attributes
    // TODO: on some ext attributes need to have more checks

    if ( type != "device")
    {
        auto ret = insert_dc_room_row_rack_group (conn, name.c_str(), type_id, parent_id, 
                extattributes, status.c_str(), priority, bc);
    }
    else
    {    auto ret = insert_device (conn, links, groups, name.c_str(), parent_id,
                    extattributes, subtype_id, status.c_str(), priority, bc);
    }
    //TODO: check from DB and call is_valid_location_chain

}


/*
 * XXX: an example how it should look like - go through a file and
 * check the values - Database will obsolete most of this
 * */
void load_asset_csv(std::istream& input)
{
    std::vector<std::vector<std::string> > data;
    cxxtools::CsvDeserializer deserializer(input);
    deserializer.delimiter(',');
    deserializer.readTitle(false);
    deserializer.deserialize(data);

    CsvMap cm{data};
    cm.deserialize();

    tntdb::Connection conn;
    try{
        conn = tntdb::connect(url);
    }
    catch(...)
    {
        return;
    }
    for (size_t row_i = 1; row_i != cm.rows(); row_i++)
    {
        try{
            process_row(conn, cm,row_i);
            //add to statistic
        }
        catch(...)
        {
        }
    }
}

} //namespace shared
