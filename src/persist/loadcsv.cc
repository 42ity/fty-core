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
#include "log.h"
#include "assetcrud.h"
#include "dbpath.h"

using namespace shared;

// check if it is valid location chain
// valid_location_chain("device", "rack") -> true
// valid_location_chain("room", "rack") -> false
static bool is_valid_location_chain(const std::string& type, const std::string& parent_type) {
    // TODO hardcoded constants
    static const std::vector<std::string> LOCATION_CHAIN = \
        {"datacenter", "room", "row", "rack", "device"};

    size_t i, ti, pi;
    i = 0;
    for (auto &x : LOCATION_CHAIN) {
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
    //TODO STATIC common
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
    LOG_START;
    log_debug ("row number is %zu", row_i);
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
    log_debug ("name = '%s'", name.c_str());
    if ( is_ok_name(name.c_str()) )
        throw std::invalid_argument("name is empty");
    unused_columns.erase("name");

    auto type = cm.get_strip(row_i, "type");
    log_debug ("type = '%s'", type.c_str());
    if ( TYPES.find(type) == TYPES.end() )
        throw std::invalid_argument("Type '" + type + "' is not allowed");
    auto type_id = TYPES.find(type)->second;
    unused_columns.erase("type");

    auto status = cm.get_strip(row_i, "status");
    log_debug ("status = '%s'", status.c_str());
    if ( STATUSES.find(status) == STATUSES.end() )
    {
        log_warning ("Status '%s'is not allowed, use default", status.c_str());
        status = "inactive";    // default
    }
    unused_columns.erase("status");

    auto bs_critical = cm.get_strip(row_i, "business_critical");
    log_debug ("bc = '%s'", bs_critical.c_str());
    if ( bs_critical != "yes" && bs_critical != "no")
    {
        log_warning ( "Business critical '%s' is not allowed, use default", bs_critical.c_str());
        bs_critical = "no";
    }
    unused_columns.erase("business_critical");
    // TODO function
    int bc = 1;
    if (bs_critical == "no")
        bc = 0;

    int priority = get_priority(cm.get_strip(row_i, "priority"));
    log_debug ("priority = %d", priority);
    unused_columns.erase("priority");

    auto location = cm.get(row_i, "location");
    log_debug ("location = '%s'", location.c_str());
    a_elmnt_id_t parent_id = 0;
    if ( !location.empty() )
    {
        auto ret = select_asset_element_by_name(conn, location.c_str());
        if ( ret.status == 1 )
            parent_id = ret.item.id;
        else
        {
            throw std::invalid_argument( "Location '" + location 
                            + "' is not present in DB");
        }
    }
    unused_columns.erase("location");
    
    auto subtype = cm.get_strip(row_i, "sub_type");
    log_debug ("subtype = '%s'", subtype.c_str());
    if ( ( type == "device" ) && ( SUBTYPES.find(subtype) == SUBTYPES.end() ) ) {
        throw std::invalid_argument ("Subtype '" + subtype + "' is not allowed");
    }
    if ( ( !subtype.empty() ) && ( type != "device" ) && ( type != "group") )
    {
        // TODO LOG
        log_warning ("'%s' - subtype is ignored", subtype.c_str());
    }
    auto subtype_id = TYPES.find(type)->second;
    unused_columns.erase("sub_type");

    
    std::set <a_elmnt_id_t>  groups{}; // list of element id of all groups, the element belongs to   
    for ( int group_index = 1 ; true; group_index++ )
    {
        try {
            auto grp_col_name = "group." + std::to_string(group_index);   // column name
            unused_columns.erase(grp_col_name);                           // remove from unused
            auto group = cm.get(row_i, grp_col_name);                     // take value
            log_debug ("group_name = '%s'", group.c_str());
            if ( !group.empty() )   // if group was not specified, just skip it
            {
                auto ret = select_asset_element_by_name(conn, group.c_str()); // find an id from DB 
                if ( ret.status == 1 )                                          
                    groups.insert(ret.item.id);  // if OK, then take ID
                else
                {
                    // TODO LOG
                    log_warning ("'%s' - the group was ignored, because doesn't exist in DB", group.c_str());
                }
            }
        }
        catch (const std::out_of_range &e) // if column doesn't exist, then break the cycle
        {
            log_debug ("end of group processing");
            log_debug (e.what());
            break;
        }
    }
    
    std::vector <link_t>  links{};
    for ( int link_index = 1; true; link_index++ )
    {
        link_t one_link{0, 0, NULL, NULL, 0};
        try {
            auto link_col_name = "power_source." + std::to_string(link_index);   // column name
            unused_columns.erase(link_col_name);                           // remove from unused
            auto link_source = cm.get(row_i, link_col_name);               // take value
            log_debug ("power_source_name = '%s'", link_source.c_str());
            if ( !link_source.empty() ) // if power source is not specified
            {
                auto ret = select_asset_element_by_name(conn, link_source.c_str());   // find an id from DB 
                if ( ret.status == 1 )                                          
                    one_link.src = ret.item.id;  // if OK, then take ID
                else
                {
                    // TODO LOG
                    log_warning ("'%s' - the unknown power source, all information would be ignored (doesn't exist in DB)", link_col_name.c_str());
                }
            }
        }
        catch (const std::out_of_range &e) // if column doesn't exist, then break the cycle
        {
            log_debug ("end of power links processing");
            log_debug (e.what());
            break;
        }
        
        auto link_col_name1 = "power_plug_src." + std::to_string(link_index);   // column name
        try{   
            unused_columns.erase(link_col_name1);                                   // remove from unused
            auto link_source1 = cm.get(row_i, link_col_name1);               // take value
            // TODO: bad idea, char = byte
            one_link.src_out = new char [4];
            strcpy ( one_link.src_out, link_source1.substr (0,4).c_str());
        }
        catch (const std::out_of_range &e) // if column doesn't exist, then break the cycle
        {
            log_debug ("'%s' - is missing at all", link_col_name1.c_str());
            log_debug (e.what());
        }
        
        auto link_col_name2 = "power_input." + std::to_string(link_index);   // column name
        try{
            unused_columns.erase(link_col_name2);                            // remove from unused
            auto link_source2 = cm.get(row_i, link_col_name2);               // take value
            // TODO: bad idea, char = byte
            one_link.dest_in = new char [4];
            strcpy ( one_link.dest_in, link_source2.substr (0,4).c_str());
        }
        catch (const std::out_of_range &e) // if column doesn't exist, then break the cycle
        {
            log_debug ("'%s' - is missing at all", link_col_name2.c_str());
            log_debug (e.what());
        }

        if ( one_link.src != 0 ) // if first column was ok
        {
            // TODO
            one_link.type = 1; // TODO remove hardcoded constant
            // TODO smth is wrong here
            links.push_back(one_link);
        }
    }
    zhash_t *extattributes = zhash_new();
    zhash_autofree(extattributes);
    for ( auto &key: unused_columns )
    {
        // try is not needed, because here are keys that are defenitly there
        auto value = cm.get(row_i, key);
        // TODO: on some ext attributes need to have more checks
        if ( !value.empty() )
            zhash_insert (extattributes, key.c_str(), (void*)value.c_str());
    }

    if ( type != "device" )
    {
        // this is a transaction
        auto ret = insert_dc_room_row_rack_group (conn, name.c_str(), type_id, parent_id, 
                extattributes, status.c_str(), priority, bc);
        if ( ret.status != 1 )
        {
            throw std::invalid_argument("insertion was unsuccess");
        }
    }
    else
    {   
        // this is a transaction 
        auto ret = insert_device (conn, links, groups, name.c_str(), parent_id,
                    extattributes, subtype_id, status.c_str(), priority, bc);
        if ( ret.status != 1 )
        {
            throw std::invalid_argument("insertion was unsuccess");
        }

    }
    LOG_END;
    //TODO: check from DB and call is_valid_location_chain
}


static bool mandatory_present (CsvMap cm)
{
    auto all_fields = cm.getTitles();
    if ( (all_fields.count("name") == 0 ) ||
         (all_fields.count("type") == 0 ) ||
         (all_fields.count("location") == 0) )
        return false;
    else
        return true;
}

void
    load_asset_csv
        (std::istream& input)
{
    LOG_START;
    std::vector <std::vector<std::string> > data;
    cxxtools::CsvDeserializer deserializer(input);
    // TODO make it configurable
    deserializer.delimiter('\t');
    deserializer.readTitle(false);
    deserializer.deserialize(data);
    log_debug ("first");
    CsvMap cm{data};
    log_debug ("first1");
    cm.deserialize();
    log_debug ("first2");

    if ( !mandatory_present(cm) )
    {
        log_error ("mandatory columns are not present, import is aborted");
        return;
    }

    tntdb::Connection conn;
    try{
        conn = tntdb::connect(url);
    }
    catch(...)
    {
        log_error ("no connection to database");
        return;
    }
    
    for (size_t row_i = 1; row_i != cm.rows(); row_i++)
    {
        try{
            process_row(conn, cm, row_i);
            // TODO LOG
            log_info ("row %zu was imported successfully", row_i);
        }
        catch ( const std::invalid_argument &e)
        {
            // TODO
            log_error ("%s", e.what());
            // log_somewhere an error
        }
    }
}

