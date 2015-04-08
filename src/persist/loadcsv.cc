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

/*! \file  loadcsv.cc
    \brief Implementation of csv import into bios
    \author Michal Vyskocil   <michalvyskocil@eaton.com>
            Alena  Chernikava <alenachernikava@eaton.com>
*/
#include <tntdb/connect.h>
#include "loadcsv.h"

#include "csv.h"
#include "assetcrud.h"
#include "dbpath.h"

using namespace shared;

//TODO TODO TODO

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
    // in case of any error, it would be empty
    return res.item;
}

static std::map<std::string,int> read_device_types(tntdb::Connection &conn)
{
    auto res = get_dictionary_device_type(conn);
    // in case of any error, it would be empty
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

    // This is used to track, what columns we had already proceeded,
    // because if we didn't proceed it yet, then it should treated as external attribute
    auto unused_columns = cm.getTitles();

    auto name = cm.get(row_i, "name");
    if ( !name.empty() )
        throw std::invalid_argument("name is empty");
    unused_columns.erase("name");

    auto type = cm.get_strip(row_i, "type");
    if ( TYPES.find(type) == TYPES.end() )
        throw std::invalid_argument("Type '" + type + "' is not allowed");
    auto type_id = TYPES.find(type)->second;
    unused_columns.erase("type");

    auto status = cm.get_strip(row_i, "status");
    if ( STATUSES.find(status) == STATUSES.end() )
    {
        throw std::invalid_argument( "Status '" + status + "' is not allowed");
        // OR
        // status = "nonactive";
    }
    unused_columns.erase("status");

    auto bs_critical = cm.get_strip(row_i, "business_critical");
    if (bs_critical != "yes" && bs_critical != "no") {
        throw std::invalid_argument( "Business critical '" + status 
                            + "' is not allowed");
        // OR
        // bs_critical = "no";
    }
    unused_columns.erase("business_critical");
    // TODO function
    int bc = 1;
    if (bs_critical == "no")
        bc = 0;

    int priority = get_priority(cm.get_strip(row_i, "priority"));
    unused_columns.erase("priority");

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
    unused_columns.erase("location");
    
    auto subtype = cm.get_strip(row_i, "subtype");
    if ( ( type == "device" ) && ( SUBTYPES.find(subtype) == SUBTYPES.end() ) ) {
        throw std::invalid_argument ("Subtype '" + subtype + "' is not allowed");
    }
    if ( ( type != "device" ) && ( type != "group") )
    {
        // need to write somewhere, that this column would be ignored
    }
    auto subtype_id = TYPES.find(type)->second;
    unused_columns.erase("subtype");

    
    std::set <a_elmnt_id_t>  groups{}; // list of element id of all groups, the element belongs to   
    for (int group_index = 1 ; true; group_index++ )
    {
        try {
            auto grp_col_name = "group." + std::to_string(group_index);   // column name
            unused_columns.erase(grp_col_name);                           // remove from unused
            auto group = cm.get(row_i, grp_col_name);                     // take value
            auto ret = select_asset_element_by_name(conn, group.c_str()); // find an id from DB 
            if ( ret.status == 1 )                                          
                groups.insert(ret.item.id);  // if OK, then take ID
            else
            {
                //LOG this group would be ignored
            }
        }
        catch (...) // if column doesn't exists, then reak the cycle
        {
            break;
        }
    }
    
    std::set <link_t>  links{};
    // TODO read and fill this

    zhash_t *extattributes = zhash_new();
    zhash_autofree(extattributes);
    for ( auto &key: unused_columns )
    {
        auto value = cm.get(row_i, key);
        // TODO: on some ext attributes need to have more checks
        zhash_insert (extattributes, key.c_str(), (void*)value.c_str());
    }

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
void
    load_asset_csv
        (std::istream& input)
{
    std::vector <std::vector<std::string> > data;
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
    // TODO != ????
    for (size_t row_i = 1; row_i != cm.rows(); row_i++)
    {
        try{
            process_row(conn, cm,row_i);
            //log_somewhere, that it is was successfull
        }
        catch(...)
        {
            // log_somewhere an error
        }
    }
}

