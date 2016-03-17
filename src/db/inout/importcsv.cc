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

#include <string>
#include <algorithm>
#include <ctype.h>

#include <tntdb/connect.h>
#include <cxxtools/regex.h>
#include <cxxtools/join.h>
#include <cxxtools/trim.h>

#include "db/inout.h"

#include "log.h"
#include "assetcrud.h"
#include "dbpath.h"
#include "cleanup.h"
#include "db/asset_general.h"
#include "db/assets.h"
#include "db/inout.h"
#include "utils.h"
#include "utils++.h"
#include "utils_web.h"

using namespace shared;

namespace persist {
int
    get_priority
        (const std::string& s)
{
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
    check_u_size
        (std::string &s)
{
    static cxxtools::Regex regex("^[0-9]{1,2}[uU]?$");
    if ( !regex.match(s) )
    {
        return false;
    }
    else
    {
        // need to drop the "U"
        if ( ! (::isdigit(s.back())) )
        {
            s.pop_back();
        }
        // remove trailing 0
        if (s.size() == 2 && s[0] == '0') {
            s.erase(s.begin());
        }
        return true;
    }
}

static bool
    match_ext_attr
        (std::string &value, const std::string &key)
{
    if ( key == "location_u_pos" )
    {
        return check_location_u_pos(value);
    }
    if ( key == "u_size" )
    {
        return check_u_size(value);
    }
    return ( !value.empty() );
}

// check if key contains date
// ... warranty_end or ends with _date
static bool
    is_date (const std::string &key)
{
    static cxxtools::Regex regex{"(^warranty_end$|_date$)"};
    return regex.match (key);
}

/*
 * \brief Processes a single row from csv file
 *
 * \param[in] conn     - a connection to DB
 * \param[in] cm       - already parsed csv file
 * \param[in] row_i    - number of row to process
 * \param[in] TYPES    - list of available types
 * \param[in] SUBTYPES - list of available subtypes
 * \param[in][out] ids - list of already seen asset ids
 *
 */
static std::pair<db_a_elmnt_t, persist::asset_operation>
    process_row
        (tntdb::Connection &conn,
         const CsvMap &cm,
         size_t row_i,
         const std::map<std::string,int> &TYPES,
         const std::map<std::string,int> &SUBTYPES,
         std::set<a_elmnt_id_t> &ids
         )
{
    LOG_START;

    log_debug ("################ Row number is %zu", row_i);
    static const std::set<std::string> STATUSES = \
        {"active", "nonactive", "spare", "retired"};

    // This is used to track, which columns had been already processed,
    // because if they was't processed yet,
    // then they should be treated as external attributes
    auto unused_columns = cm.getTitles();

    if (unused_columns.empty()) {
        bios_throw("bad-request-document", "Cannot import empty document.");
    }

    // because id is definitely not an external attribute
    auto id_str = unused_columns.count("id") ? cm.get(row_i, "id") : "";
    unused_columns.erase("id");
    persist::asset_operation operation = persist::asset_operation::INSERT;
    a_elmnt_id_t id = 0;
    if ( !id_str.empty() )
    {
        id = atoi(id_str.c_str());
        if ( ids.count(id) == 1 ) {
            std::string msg = "Element id '";
            msg += id_str;
            msg += "' found twice, aborting";
            bios_throw("bad-request-document", msg.c_str());
        }
        ids.insert(id);
        operation = persist::asset_operation::UPDATE;
    }

    auto name = cm.get(row_i, "name");
    log_debug ("name = '%s'", name.c_str());
    if ( !is_ok_name(name.c_str()) ) {
        bios_throw("request-param-bad", "name", name.empty() ? "<empty>" : name.c_str(), "<unique and non empty value>");
    }
    unused_columns.erase("name");

    auto type = cm.get_strip(row_i, "type");
    log_debug ("type = '%s'", type.c_str());
    if ( TYPES.find(type) == TYPES.end() ) {
        bios_throw("request-param-bad", "type", type.empty() ? "<empty>" : type.c_str(), utils::join_keys_map(TYPES, ", ").c_str());
    }
    auto type_id = TYPES.find(type)->second;
    unused_columns.erase("type");

    auto status = cm.get_strip(row_i, "status");
    log_debug ("status = '%s'", status.c_str());
    if ( STATUSES.find(status) == STATUSES.end() ) {
        bios_throw ("request-param-bad", "status", status.empty() ? "<empty>" : status.c_str(),
            cxxtools::join(STATUSES.cbegin(), STATUSES.cend(), ", ").c_str());
    }
    unused_columns.erase("status");

    auto asset_tag =  unused_columns.count("asset_tag") ? cm.get(row_i, "asset_tag") : "";
    log_debug ("asset_tag = '%s'", asset_tag.c_str());
    if ( ( !asset_tag.empty() ) && ( asset_tag.length() > 50 ) ){
        bios_throw("request-param-bad", "asset_tag", "<to long>", "<unique string from 1 to 50 characters>");
    }
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
        else {
            bios_throw("request-param-bad", "location", location.c_str(), "<existing asset name>");
        }
    }
    unused_columns.erase("location");
    
    // Business requirement: be able to write 'rack controller', 'RC', 'rc' as subtype == 'rack controller'
    std::map<std::string,int> local_SUBTYPES = SUBTYPES;
    int rack_controller_id = SUBTYPES.find ("rack controller")->second;

    local_SUBTYPES.emplace (std::make_pair ("rackcontroller", rack_controller_id));
    local_SUBTYPES.emplace (std::make_pair ("rc", rack_controller_id));
    local_SUBTYPES.emplace (std::make_pair ("RC", rack_controller_id));
    local_SUBTYPES.emplace (std::make_pair ("RC3", rack_controller_id));

    auto subtype = cm.get (row_i, "sub_type");
    cxxtools::ltrim (subtype);
    cxxtools::rtrim (subtype);

    log_debug ("subtype = '%s'", subtype.c_str());
    if ( ( type == "device" ) &&
         ( local_SUBTYPES.find(subtype) == local_SUBTYPES.cend() ) ) {
        bios_throw("request-param-bad", "subtype", subtype.empty() ? "<empty>" : subtype.c_str(), utils::join_keys_map(SUBTYPES, ", ").c_str());
    }

    if ( ( !subtype.empty() ) && ( type != "device" ) && ( type != "group") )
    {
        log_warning ("'%s' - subtype is ignored", subtype.c_str());
    }

    if ( ( subtype.empty() ) && ( type == "group" ) ) {
        bios_throw("request-param-required", "subtype (for type group)");
    }

    auto subtype_id = local_SUBTYPES.find(subtype)->second;
    unused_columns.erase("sub_type");

    // now we have read all basic information about element
    // if id is set, then it is right time to check what is going on in DB
    if ( !id_str.empty() )
    {
        db_reply <db_web_basic_element_t> element_in_db = select_asset_element_web_byId
                                                        (conn, id);
        if ( element_in_db.status == 0 ) {
            bios_throw("element-not-found", id_str.c_str());
        }
        else
        {
            if ( element_in_db.item.name != name ) {
                bios_throw("bad-request-document", "Renaming of asset is not supported");
            }
            if ( element_in_db.item.type_id != type_id ) {
                bios_throw("bad-request-document", "Changing of asset type is forbidden");
            }
            if ( ( element_in_db.item.subtype_id != subtype_id ) &&
                 ( element_in_db.item.subtype_name != "N_A" ) ) {
                bios_throw("bad-request-document", "Changing of asset subtype is forbidden");
            }
        }
    }

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
                bios_throw("element-not-found", group.c_str());
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
                bios_throw("element-not-found", link_source.c_str());
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
            // FIXME: THIS IS MEMORY LEAK!!!
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
            // FIXME: THIS IS MEMORY LEAK!!!
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
        std::string value = cm.get(row_i, key);

        // BIOS-1564: sanitize the date for warranty_end -- start
        if (is_date (key) && !value.empty()) {
            char *date = sanitize_date (value.c_str());
            if (!date) {
                log_error ("Cannot sanitize %s '%s' for device '%s'", key.c_str(), value.c_str(), name.c_str());
                bios_throw("request-param-bad", key.c_str(), value.c_str(), "ISO date");
            }
            value = date;
            zstr_free (&date);
        }
        // BIOS-1564 -- end

        if ( match_ext_attr (value, key) )
        {
            if ( key == "serial_no" )
            {

                if  ( unique_keytag (conn, key, value, id) == 0 )
                    zhash_insert (extattributes, key.c_str(), (void*)value.c_str());
                else
                {
                    bios_throw("request-param-bad", "serial_no", value.c_str(), "<unique string>");
                }
                continue;
            }
            zhash_insert (extattributes, key.c_str(), (void*)value.c_str());
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
                 extattributes, status.c_str(), priority, groups, asset_tag, errmsg);
            if ( ( ret ) || ( !errmsg.empty() ) )
            {
                //TODO: redo the update_dc_room_row_rack_group
                throw std::invalid_argument(errmsg);
            }
        }
        else
        {
            auto ret = update_device
                (conn, m.id, name.c_str(), type_id, parent_id,
                 extattributes, status.c_str(), priority, groups, links, asset_tag, errmsg);
            if ( ( ret ) || ( !errmsg.empty() ) )
            {
                //TODO: redo the update_dc_room_row_rack_group
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
                 extattributes, status.c_str(), priority, groups, asset_tag);
            if ( ret.status != 1 )
            {
                throw BiosError(ret.rowid, ret.msg);
            }
            m.id = ret.rowid;
        }
        else
        {
            // this is a transaction
            auto ret = insert_device (conn, links, groups, name.c_str(),
                    parent_id, extattributes, subtype_id, subtype.c_str(), status.c_str(),
                    priority, asset_tag);
            if ( ret.status != 1 )
            {
                throw BiosError(ret.rowid, ret.msg);
            }
            m.id = ret.rowid;
        }
    }
    m.name = name;
    m.status = status;
    m.parent_id = parent_id;
    m.priority = priority;
    m.type_id = type_id;
    m.subtype_id = subtype_id;
    m.asset_tag = asset_tag;

    for (void* it = zhash_first (extattributes); it != NULL;
               it = zhash_next (extattributes)) {
        m.ext.emplace (zhash_cursor (extattributes), (char*) it);
    }

    LOG_END;
    return std::make_pair(m, operation) ;
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
//MVY: moved out to support friendly error messages below
static std::vector<std::string> MANDATORY = {
    "name", "type", "sub_type", "location", "status",
    "priority"
};
static std::string
mandatory_missing
        (const CsvMap &cm)
{
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
         std::vector <std::pair<db_a_elmnt_t,persist::asset_operation>> &okRows,
         std::map <int, std::string> &failRows,
         touch_cb_t touch_fn
         )
{
    LOG_START;

    std::vector <std::vector<cxxtools::String> > data;
    cxxtools::CsvDeserializer deserializer(input);
    if ( hasApostrof(input) ) {
        std::string msg = "CSV file contains ' (apostrof), please remove it";
        log_error("%s\n", msg.c_str());
        LOG_END;
        bios_throw("bad-request-document", msg.c_str());
    }
    char delimiter = findDelimiter(input);
    if (delimiter == '\x0') {
        std::string msg{"Cannot detect the delimiter, use comma (,) semicolon (;) or tabulator"};
        log_error("%s", msg.c_str());
        LOG_END;
        bios_throw("bad-request-document", msg.c_str());
    }
    log_debug("Using delimiter '%c'", delimiter);
    deserializer.delimiter(delimiter);
    deserializer.readTitle(false);
    deserializer.deserialize(data);
    CsvMap cm{data};
    cm.deserialize();

    return load_asset_csv(cm, okRows, failRows, touch_fn);
}

std::pair<db_a_elmnt_t, persist::asset_operation>
    process_one_asset
        (const CsvMap& cm)
{
    LOG_START;

    auto m = mandatory_missing(cm);
    if ( m != "" )
    {
        std::string msg{"column '" + m + "' is missing, import is aborted"};
        log_error("%s", msg.c_str());
        LOG_END;
        bios_throw("request-param-required", m.c_str());
    }

    tntdb::Connection conn;
    try{
        conn = tntdb::connectCached(url);
    }
    catch(...)
    {
        std::string msg{"No connection to database"};
        log_error("%s", msg.c_str());
        LOG_END;
        bios_throw("internal-error", msg.c_str());
    }

    auto TYPES = read_element_types (conn);

    auto SUBTYPES = read_device_types (conn);

    std::set<a_elmnt_id_t> ids{};
    auto ret = process_row(conn, cm, 1, TYPES, SUBTYPES, ids);
    LOG_END;
    return ret;
}

void
    load_asset_csv
        (const CsvMap& cm,
         std::vector <std::pair<db_a_elmnt_t,persist::asset_operation>> &okRows,
         std::map <int, std::string> &failRows,
         touch_cb_t touch_fn
         )
{
    LOG_START;

    auto m = mandatory_missing(cm);
    if ( m != "" )
    {
        std::string msg{"column '" + m + "' is missing, import is aborted"};
        log_error("%s", msg.c_str());
        LOG_END;
        bios_throw("request-param-bad", m.c_str(), std::string("<missing column'").append(m).append("'>").c_str(),
            std::string("<column '").append(m).append("' is present in csv>").c_str());
    }

    tntdb::Connection conn;
    try{
        conn = tntdb::connectCached(url);
    }
    catch(...)
    {
        std::string msg{"No connection to database"};
        log_error("%s", msg.c_str());
        LOG_END;
        bios_throw("internal-error", msg.c_str());
    }

    auto TYPES = read_element_types (conn);

    auto SUBTYPES = read_device_types (conn);

    std::set<a_elmnt_id_t> ids{};
    for (size_t row_i = 1; row_i != cm.rows(); row_i++)
    {
        try{
            auto ret = process_row(conn, cm, row_i, TYPES, SUBTYPES, ids);
            touch_fn ();
            okRows.push_back (ret);
            log_info ("row %zu was imported successfully", row_i);
        }
        catch ( const std::invalid_argument &e)
        {
            failRows.insert(std::make_pair(row_i + 1, e.what()));
            log_error ("row %zu not imported: %s", row_i, e.what());
        }
    }
    LOG_END;
}

} // namespace persist
