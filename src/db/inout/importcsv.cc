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

/*! \file  importcsv.cc
    \brief Implementation of csv import
    \author Michal Vyskocil <MichalVyskocil@Eaton.com>
    \author Alena Chernikava <AlenaChernikava@Eaton.com>
*/
#include <tntdb/connect.h>
#include <cxxtools/regex.h>

#include "db/inout.h"

#include "csv.h"
#include "log.h"
#include "assetcrud.h"
#include "dbpath.h"
#include "cleanup.h"
#include "db/asset_general.h"
#include "db/assets.h"

using namespace shared;

namespace persist {
// convert input '?[1-5]' to 1-5
static int
    get_priority
        (const std::string& s)
{
    log_debug ("priority string = %s", s.c_str());
    if ( s.size() > 2 )
        return 5;
    for (int i = 0; i != 2; i++) {
        if (s[i] >= 49 && s[i] <= 53) {
            return s[i] - 48;
        }
    }
    return 5;
}

static std::map<std::string,int>
    read_element_types
        (tntdb::Connection &conn)
{
    auto res = get_dictionary_element_type(conn);
    // in case of any error, it would be empty
    return res.item;
}

static std::map<std::string,int>
    read_device_types
        (tntdb::Connection &conn)
{
    auto res = get_dictionary_device_type(conn);
    // in case of any error, it would be empty
    return res.item;
}

static bool
    check_location_u_pos
        (const std::string &s)
{
    cxxtools::Regex regex("^[0-9][0-9]?([uU][rR]|[uU])?$");
    if ( !regex.match(s) )
        return false;
    else
        return true;
}

static bool
    check_location_w_pos
        (const std::string &s)
{
    if ( ( s == "left" ) || ( s == "right" ) )
        return true;
    else
        return false;
}

static bool
    match_ext_attr
        (std::string value, std::string key)
{
    if ( key == "location_u_pos" )
    {
        return check_location_u_pos(value);
    }
    if ( key == "location_w_pos" )
    {
        return check_location_w_pos(value);
    }
    return ( !value.empty() );
}

int
    get_pdu_epdu_info_location_w_pos
        (tntdb::Connection &conn,
         a_elmnt_id_t parent_id,
         int &pdu_epdu_count,
         std::string &last_position)
{
    LOG_START;
    db_reply <std::vector<device_info_t>> devices =
        select_asset_device_by_container
         (conn, parent_id);
    pdu_epdu_count = 0;
    for ( auto &adevice : devices.item )
    {
        if ( ( std::get<2>(adevice) == "pdu" ) || ( std::get<2>(adevice) == "epdu" ) )
        {
            pdu_epdu_count++;
            db_reply <std::map <std::string, std::pair<std::string, bool> >>
                ext_attributes = persist::select_ext_attributes (conn, std::get<0>(adevice));
            auto it = ext_attributes.item.find("location_w_pos");
            if ( it != ext_attributes.item.cend() )
                last_position = it->second.first;
            else
            {
                log_warning ("inconsistent state: epdu/pdu without location_w_pos, use 'right'");
                last_position = "left";
            }
        }
    }
    LOG_END;
    return 0;
}

/*
 * \brief Processes a single row from csv file
 *
 * \param conn - a connection to DB
 * \param cm - already parsed csv file
 * \param row_i - number of row to process
 */
static db_a_elmnt_t
    process_row
        (tntdb::Connection &conn,
         CsvMap cm,
         size_t row_i,
         std::map<std::string,int> TYPES,
         std::map<std::string,int> SUBTYPES
         )
{
    LOG_START;

    log_debug ("row number is %zu", row_i);
    static const std::set<std::string> STATUSES = \
        {"active", "nonactive", "spare", "retired"};

    // This is used to track, which columns had been already processed,
    // because if they was't processed yet,
    // then they should be treated as external attributes
    auto unused_columns = cm.getTitles();

    // because id is definitely is not an ext attribute
    auto id_str = unused_columns.count("id") ? cm.get(row_i, "id") : "";
    unused_columns.erase("id");


    auto name = cm.get(row_i, "name");
    log_debug ("name = '%s'", name.c_str());
    if ( !is_ok_name(name.c_str()) )
        throw std::invalid_argument("name is not valid");
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
        log_warning ("Status '%s' is not allowed, use default",
                                                            status.c_str());
        status = "nonactive";    // default
    }
    unused_columns.erase("status");

    auto bs_critical = cm.get_strip(row_i, "business_critical");
    log_debug ("bc = '%s'", bs_critical.c_str());
    if ( bs_critical != "yes" && bs_critical != "no")
    {
        log_warning ( "Business critical '%s' is not allowed, use default",
                                                        bs_critical.c_str());
        bs_critical = "no";
    }
    unused_columns.erase("business_critical");
    // TODO function
    int bc = 1;
    if (bs_critical == "no")
        bc = 0;

    auto asset_tag = cm.get(row_i, "asset_tag");
    log_debug ("asset_tag = '%s'", asset_tag.c_str());
    if ( asset_tag.empty() )
        throw std::invalid_argument("asset_tag is required");
    if ( asset_tag.length() < 6 )
        throw std::invalid_argument("asset_tag should have at least 6 characters");
    if ( asset_tag.length() > 10 )
        throw std::invalid_argument("asset_tag should have max 10 characters");
    unused_columns.erase("asset_tag");

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
    if ( ( type == "device" ) &&
         ( SUBTYPES.find(subtype) == SUBTYPES.end() ) )
    {
        throw std::invalid_argument
                                ("Subtype '" + subtype + "' is not allowed");
    }
    if ( ( !subtype.empty() ) && ( type != "device" ) && ( type != "group") )
    {
        log_warning ("'%s' - subtype is ignored", subtype.c_str());
    }
    if ( ( subtype.empty() ) && ( type == "group" ) )
        throw std::invalid_argument
                                ("Subtype is mandatory for the group");

    auto subtype_id = SUBTYPES.find(subtype)->second;
    unused_columns.erase("sub_type");

    // BIOS-991 --start
    int pdu_epdu_count = 0;
    std::string last_position = "";
    if ( ( subtype == "epdu" ) || ( subtype == "pdu" ) )
    {
        // find a number of pdu/epdu in the rack
        int ret = get_pdu_epdu_info_location_w_pos (conn, parent_id, pdu_epdu_count, last_position);
        if ( ret )
            throw std::invalid_argument
                                ("some problem with db, see log for more details");
        if ( ( pdu_epdu_count > 1 ) && ( id_str.empty() ) )
            throw std::invalid_argument
                                ("more than 2 PDU is not supported");
    }
    // BIOS-991 --end

    std::string group;

    // list of element ids of all groups, the element belongs to
    std::set <a_elmnt_id_t>  groups{};
    for ( int group_index = 1 ; true; group_index++ )
    {
        std::string grp_col_name = "";
        try {
            // column name
            grp_col_name = "group." + std::to_string(group_index);
            // remove from unused
            unused_columns.erase(grp_col_name);
            // take value
            group = cm.get(row_i, grp_col_name);
        }
        catch (const std::out_of_range &e)
        // if column doesn't exist, then break the cycle
        {
            log_debug ("end of group processing");
            log_debug (e.what());
            break;
        }
        log_debug ("group_name = '%s'", group.c_str());
        // if group was not specified, just skip it
        if ( !group.empty() )
        {
            // find an id from DB
            auto ret = select_asset_element_by_name(conn, group.c_str());
            if ( ret.status == 1 )
                groups.insert(ret.item.id);  // if OK, then take ID
            else
            {
                log_error ("group '%s' is not present in DB, rejected", group.c_str());
                throw std::invalid_argument
                    ("The group " + group + " is not present in DB");
            }
        }
    }

    std::vector <link_t>  links{};
    std::string  link_source = "";
    for ( int link_index = 1; true; link_index++ )
    {
        link_t one_link{0, 0, NULL, NULL, 0};
        std::string link_col_name = "";
        try {
            // column name
            link_col_name = "power_source." +
                                                std::to_string(link_index);
            // remove from unused
            unused_columns.erase(link_col_name);
            // take value
            link_source = cm.get(row_i, link_col_name);
        }
        catch (const std::out_of_range &e)
        // if column doesn't exist, then break the cycle
        {
            log_debug ("end of power links processing");
            log_debug (e.what());
            break;
        }

        log_debug ("power_source_name = '%s'", link_source.c_str());
        if ( !link_source.empty() ) // if power source is not specified
        {
            // find an id from DB
            auto ret = select_asset_element_by_name
                (conn, link_source.c_str());
            if ( ret.status == 1 )
                one_link.src = ret.item.id;  // if OK, then take ID
            else
            {
                log_warning ("power source '%s' is not present in DB, rejected",
                    link_source.c_str());
                throw std::invalid_argument
                    ("The power source " + link_source + " is not present in DB");
            }
        }

        // column name
        auto link_col_name1 = "power_plug_src." + std::to_string(link_index);
        try{
            // remove from unused
            unused_columns.erase(link_col_name1);
            // take value
            auto link_source1 = cm.get(row_i, link_col_name1);
            // TODO: bad idea, char = byte
            one_link.src_out = new char [4];
            strcpy ( one_link.src_out, link_source1.substr (0,4).c_str());
        }
        catch (const std::out_of_range &e)
        {
            log_debug ("'%s' - is missing at all", link_col_name1.c_str());
            log_debug (e.what());
        }

        // column name
        auto link_col_name2 = "power_input." + std::to_string(link_index);
        try{
            unused_columns.erase(link_col_name2); // remove from unused
            auto link_source2 = cm.get(row_i, link_col_name2);// take value
            // TODO: bad idea, char = byte
            one_link.dest_in = new char [4];
            strcpy ( one_link.dest_in, link_source2.substr (0,4).c_str());
        }
        catch (const std::out_of_range &e)
        {
            log_debug ("'%s' - is missing at all", link_col_name2.c_str());
            log_debug (e.what());
        }

        if ( one_link.src != 0 ) // if first column was ok
        {
            if ( type == "device" )
            {
                one_link.type = 1; // TODO remove hardcoded constant
                links.push_back(one_link);
            }
            else
            {
                log_warning ("information about power sources is ignored for type '%s'", type.c_str());
            }
        }
    }
    _scoped_zhash_t *extattributes = zhash_new();
    zhash_autofree(extattributes);
    for ( auto &key: unused_columns )
    {
        // try is not needed, because here are keys that are definitely there
        auto value = cm.get(row_i, key);

        if ( match_ext_attr (value, key) )
        {
            // may be lambda can perfectly be used here
            if ( ( ( subtype == "pdu" ) || ( subtype == "epdu" ) ) && ( key == "location_w_pos" ) )
            {
                // BIOS-991 --start
                switch ( pdu_epdu_count ) {
                    case 0: {
                                zhash_insert (extattributes, key.c_str(), (void*)value.c_str());
                                break;
                            }
                    case 1: {
                                std::string new_value = ( last_position == "left" ) ? "right" : "left";
                                if ( new_value != value )
                                    log_warning (" location_w_pow changed to '%s'", new_value.c_str());
                                zhash_insert (extattributes, key.c_str(), (void*)new_value.c_str());
                                break;
                            }
                    default:
                            {
                                throw std::invalid_argument
                                    ("this should never happen: position of pdu/epdu");
                            }
                }
                // BIOS-991 --end
                continue;
            }
            if ( key == "serial_no" )
            {
                a_elmnt_id_t  id = 0;
                if ( !id_str.empty() )
                    id = atoi(id_str.c_str());

                if  ( unique_keytag (conn, key, value, id) == 0 )
                    zhash_insert (extattributes, key.c_str(), (void*)value.c_str());
                else
                {
                    std::string errmsg = "serial_no '" + value + "' is already used";
                    throw std::invalid_argument(errmsg);
                }
                continue;
            }
            zhash_insert (extattributes, key.c_str(), (void*)value.c_str());
        }
        else
        {
            if ( ( ( subtype == "pdu" ) || ( subtype == "epdu" ) ) && ( key == "location_w_pos" ) )
            {
                throw std::invalid_argument
                    ("location_w_pos should be set. Possible variants 'left'/'right'");
            }
            log_debug ("key = %s value = %s was ignored", key.c_str(), value.c_str());
        }
    }
    // if the row represents group, the subtype represents a type
    // of the group.
    // As group has no special table as device, then this information
    // sould be inserted as external attribute
    // this parametr is mandatory according rfc-11
    if ( type == "group" )
        zhash_insert (extattributes, "type", (void*) subtype.c_str() );

    db_a_elmnt_t m;

    if ( !id_str.empty() )
    {
        m.id = atoi(id_str.c_str());
        std::string errmsg = "";
        if (type != "device" )
        {
            auto ret = update_dc_room_row_rack_group
                (conn, m.id, name.c_str(), type_id, parent_id,
                 extattributes, status.c_str(), priority, bc, groups, asset_tag, errmsg);
            if ( ( ret ) || ( !errmsg.empty() ) )
            {
                throw std::invalid_argument(errmsg);
            }
        }
        else
        {
            auto ret = update_device
                (conn, m.id, name.c_str(), type_id, parent_id,
                 extattributes, status.c_str(), priority, bc, groups, links, asset_tag, errmsg);
            if ( ( ret ) || ( !errmsg.empty() ) )
            {
                throw std::invalid_argument(errmsg);
            }
        }

    }
    else
    {
        if ( type != "device" )
        {
            // this is a transaction
            auto ret = insert_dc_room_row_rack_group
                (conn, name.c_str(), type_id, parent_id,
                 extattributes, status.c_str(), priority, bc, groups, asset_tag);
            if ( ret.status != 1 )
            {
                throw std::invalid_argument("insertion was unsuccessful");
            }
        }
        else
        {
            // this is a transaction
            auto ret = insert_device (conn, links, groups, name.c_str(),
                    parent_id, extattributes, subtype_id, subtype.c_str(), status.c_str(),
                    priority, bc, asset_tag);
            if ( ret.status != 1 )
            {
                throw std::invalid_argument("insertion was unsuccessful");
            }
        }
    }
    m.name = name;
    m.status = status;
    m.parent_id = parent_id;
    m.priority = priority;
    m.bc = bc;
    m.type_id = type_id;
    m.subtype_id = subtype_id;
    m.asset_tag = asset_tag;

    LOG_END;
    return m;
}


/*
 * \brief Checks if mandatory columns are missing in csv file
 *
 * This check is implemented according BAM DC010
 *
 * \param cm - already parsed csv file
 *
 * \return emtpy string if everything is ok, otherwise the name of missing row
 */
static std::string
mandatory_missing
        (CsvMap cm)
{
    static std::vector<std::string> MANDATORY = {
        "name", "type", "sub_type", "location", "status",
        "business_critical", "priority", "asset_tag"
    };

    auto all_fields = cm.getTitles();
    for (const auto& s : MANDATORY) {
        if (all_fields.count(s) == 0)
            return s;
    }

    return "";
}


void
    load_asset_csv
        (std::istream& input,
         std::vector <db_a_elmnt_t> &okRows,
         std::map <int, std::string> &failRows)
{
    LOG_START;

    std::vector <std::vector<cxxtools::String> > data;
    cxxtools::CsvDeserializer deserializer(input);
    char delimiter = findDelimiter(input);
    if (delimiter == '\x0') {
        std::string msg{"Cannot detect the delimiter, use comma (,) semicolon (;) or tabulator"};
        log_error("%s\n", msg.c_str());
        LOG_END;
        throw std::invalid_argument(msg.c_str());
    }
    log_debug("Using delimiter '%c'", delimiter);
    deserializer.delimiter(delimiter);
    deserializer.readTitle(false);
    deserializer.deserialize(data);
    CsvMap cm{data};
    cm.deserialize();

    auto m = mandatory_missing(cm);
    if ( m != "" )
    {
        std::string msg{"column '" + m + "' is missing, import is aborted"};
        log_error("%s", msg.c_str());
        LOG_END;
        throw std::invalid_argument(msg.c_str());
    }

    tntdb::Connection conn;
    try{
        conn = tntdb::connectCached(url);
    }
    catch(...)
    {
        std::string msg{"no connection to database"};
        log_error("%s", msg.c_str());
        LOG_END;
        throw std::runtime_error(msg.c_str());
    }

    auto TYPES = read_element_types (conn);

    auto SUBTYPES = read_device_types (conn);

    for (size_t row_i = 1; row_i != cm.rows(); row_i++)
    {
        try{
            auto ret = process_row(conn, cm, row_i, TYPES, SUBTYPES);
            okRows.push_back (ret);
            log_info ("row %zu was imported successfully", row_i);
        }
        catch ( const std::invalid_argument &e)
        {
            failRows.insert(std::make_pair(row_i, e.what()));
            log_error ("row %zu not imported: %s", row_i, e.what());
        }
    }
    LOG_END;
}

} // namespace persist
