/*
Copyright (C) 2014-2015 Eaton

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

// ATTENTION: there is no easy way of getting last deleted id,
// and there is no requirements to do this.
// Then for every succesfull delete statement
// 0 would be return as rowid

#include "assetr.h"

#include <exception>
#include <assert.h>

#include <tntdb/row.h>
#include <tntdb/result.h>
#include <tntdb/error.h>

#include "log.h"
#include "defs.h"


namespace persist{

db_reply <db_web_basic_element_t>
    select_asset_element_web_byId
        (tntdb::Connection &conn,
         a_elmnt_id_t element_id)
{
    LOG_START;
    log_debug ("element_id = %" PRIi32, element_id);

    // TODO write function new
    db_web_basic_element_t item {0, "", "", 0, 0, 0, "", 0, 0, 0, "",""};
    db_reply <db_web_basic_element_t> ret = db_reply_new(item);

    try{
        // Can return more than one row.
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "   v.id, v.name, v.id_type, v.type_name,"
            "   v.subtype_id, v.subtype_name, v.id_parent,"
            "   v.id_parent_type, v.business_crit, v.status,"
            "   v.priority, v.asset_tag"
            " FROM"
            "   v_web_element v"
            " WHERE :id = v.id"
        );

        tntdb::Row row = st.set("id", element_id).
                            selectRow();
        log_debug("[v_web_element]: were selected %" PRIu32 " rows", 1);

        row[0].get(ret.item.id);
        row[1].get(ret.item.name);
        row[2].get(ret.item.type_id);
        row[3].get(ret.item.type_name);
        row[4].get(ret.item.subtype_id);
        row[5].get(ret.item.subtype_name);
        row[6].get(ret.item.parent_id);
        row[7].get(ret.item.parent_type_id);
        row[8].get(ret.item.bc);
        row[9].get(ret.item.status);
        row[10].get(ret.item.priority);
        row[11].get(ret.item.asset_tag);

        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const tntdb::NotFound &e) {
        ret.status        = 0;
        ret.errtype       = DB_ERR;
        ret.errsubtype    = DB_ERROR_NOTFOUND;
        ret.msg           = "element with spesified id was not found";
        log_info ("end: %s", ret.msg.c_str());
        return ret;
    }
    catch (const std::exception &e) {
        ret.status        = 0;
        ret.errtype       = DB_ERR;
        ret.errsubtype    = DB_ERROR_INTERNAL;
        ret.msg           = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}


db_reply <std::map <std::string, std::pair<std::string, bool> >>
    select_ext_attributes
        (tntdb::Connection &conn,
         a_elmnt_id_t element_id)
{
    LOG_START;
    log_debug ("element_id = %" PRIi32, element_id);

    std::map <std::string, std::pair<std::string, bool> > item{};
    db_reply <std::map <std::string, std::pair<std::string, bool> > > ret =
                                                    db_reply_new(item);
    try {
        // Can return more than one row
        tntdb::Statement st_extattr = conn.prepareCached(
            " SELECT"
            "   v.keytag, v.value, v.read_only"
            " FROM"
            "   v_bios_asset_ext_attributes v"
            " WHERE v.id_asset_element = :idelement"
        );

        tntdb::Result result = st_extattr.set("idelement", element_id).
                                          select();
        log_debug("[v_bios_asset_ext_attributes]: were selected %" PRIu32 " rows", result.size());

        // Go through the selected extra attributes
        for ( auto &row: result )
        {
            std::string keytag = "";
            row[0].get(keytag);
            std::string value = "";
            row[1].get(value);
            int read_only = 0;
            row[2].get(read_only);
            ret.item.insert (std::pair<std::string,std::pair<std::string, bool> >
                    (keytag, std::pair<std::string, bool>
                                                    (value,read_only?true:false)));
        }
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status        = 0;
        ret.errtype       = DB_ERR;
        ret.errsubtype    = DB_ERROR_INTERNAL;
        ret.msg           = e.what();
        ret.item.clear();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

int
select_ext_attributes(
        tntdb::Connection &conn,
        a_elmnt_id_t element_id,
        std::map <std::string, std::pair<std::string, bool> >& out)
{
    auto dbreply = select_ext_attributes(conn, element_id);
    if (dbreply.status == 0)
        return -1;
    out = dbreply.item;
    return 0;
}

db_reply <std::vector <db_tmp_link_t>>
    select_asset_device_links_to
        (tntdb::Connection &conn,
         a_elmnt_id_t element_id,
         a_lnk_tp_id_t link_type_id)
{
    LOG_START;
    log_debug ("element_id = %" PRIi32, element_id);

    std::vector <db_tmp_link_t> item{};
    db_reply <std::vector <db_tmp_link_t>> ret = db_reply_new(item);

    try {
        // Get information about the links the specified device
        // belongs to
        // Can return more than one row
        tntdb::Statement st_pow = conn.prepareCached(
            " SELECT"
            "   v.id_asset_element_src, v.src_out, v.dest_in"
            " FROM"
            "   v_bios_asset_link v"
            " WHERE"
            "   v.id_asset_element_dest = :iddevice AND"
            "   v.id_asset_link_type = :idlinktype"
        );

        tntdb::Result result = st_pow.set("iddevice", element_id).
                                      set("idlinktype", link_type_id).
                                      select();
        log_debug("[v_bios_asset_link]: were selected %" PRIu32 " rows", result.size());

        // Go through the selected links
        for ( auto &row: result )
        {
            db_tmp_link_t m{0, element_id, "", ""};
            row[0].get(m.src_id);
            row[1].get(m.src_socket);
            row[2].get(m.dest_socket);

            ret.item.push_back (m);
        }
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status        = 0;
        ret.errtype       = DB_ERR;
        ret.errsubtype    = DB_ERROR_INTERNAL;
        ret.msg           = e.what();
        ret.item.clear();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}


db_reply <std::vector <a_elmnt_id_t> >
    select_asset_element_groups
        (tntdb::Connection &conn,
         a_elmnt_id_t element_id)
{
    LOG_START;
    log_debug ("element_id = %" PRIi32, element_id);

    std::vector <a_elmnt_id_t> item{};
    db_reply <std::vector <a_elmnt_id_t> > ret = db_reply_new(item);

    try {
        // Get information about the groups element belongs to
        // Can return more than one row
        tntdb::Statement st_gr = conn.prepareCached(
            " SELECT"
            "   v.id_asset_group"
            " FROM"
            "   v_bios_asset_group_relation v"
            " WHERE v.id_asset_element = :idelement"
        );

        // TODO set
        tntdb::Result result = st_gr.set("idelement", element_id).
                                     select();

        log_debug("[v_bios_asset_group_relation]: were selected %" PRIu32 " rows", result.size());
        // Go through the selected groups
        for ( auto &row: result )
        {
            // group_id, required
            a_elmnt_id_t group_id = 0;
            row[0].get(group_id);
            ret.item.push_back(group_id);
        }
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status        = 0;
        ret.errtype       = DB_ERR;
        ret.errsubtype    = DB_ERROR_INTERNAL;
        ret.msg           = e.what();
        ret.item.clear();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}


db_reply <std::map <uint32_t, std::string> >
    select_short_elements
        (tntdb::Connection &conn,
         a_elmnt_tp_id_t type_id,
         a_elmnt_stp_id_t subtype_id)
{
    LOG_START;
    log_debug ("  type_id = %" PRIi16, type_id);
    log_debug ("  subtype_id = %" PRIi16, subtype_id);

    std::map <uint32_t, std::string> item{};
    db_reply <std::map <uint32_t, std::string> > ret = db_reply_new(item);

    std::string query;
    if ( subtype_id == 0 )
    {
        query = " SELECT "
                "   v.name, v.id "
                " FROM "
                "   v_bios_asset_element v "
                " WHERE "
                "   v.id_type = :typeid ";
    }
    else
    {
        query = " SELECT "
                "   v.name, v.id "
                " FROM "
                "   v_bios_asset_element v "
                " WHERE "
                "   v.id_type = :typeid AND "
                "   v.id_subtype = :subtypeid ";
    }
    try{
        // Can return more than one row.
        tntdb::Statement st = conn.prepareCached(query);

        tntdb::Result result = st.set("typeid", type_id).
                                  set("subtypeid", subtype_id).
                                  select();

        // Go through the selected elements
        for ( auto &row: result )
        {
            std::string name="";
            row[0].get(name);
            uint32_t id = 0;
            row[1].get(id);

            ret.item.insert(std::pair<uint32_t, std::string>(id, name));
        }
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status        = 0;
        ret.errtype       = DB_ERR;
        ret.errsubtype    = DB_ERROR_INTERNAL;
        ret.msg           = e.what();
        ret.item.clear();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

reply_t
    select_dc_of_asset_element
        (tntdb::Connection &conn,
         a_elmnt_id_t  element_id,
         a_elmnt_id_t &dc_id)
{
    LOG_START;
    log_debug (" element_id = %" PRIi32, element_id);

    reply_t rep;
    // TODO
    // if element is DC, then dc_id = element_id ( not implemented yet)
    try{
        // Find last parent
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "   v.id_parent5, v.id_parent4, v.id_parent3, v.id_parent2, v.id_parent1"
            " FROM"
            "   v_bios_asset_element_super_parent v"
            " WHERE v.id_asset_element = :id"
        );

        tntdb::Row row = st.set("id", element_id).
                            selectRow();
        log_debug("[v_bios_asset_element_super_parent]: were selected" \
                     "%" PRIu32 " rows",  1);
        if ( !row[0].get(dc_id) && !row[1].get(dc_id) &&
             !row[2].get(dc_id) && !row[3].get(dc_id) &&
             !row[4].get(dc_id) )
        {
            log_debug ("this element has no parent");
            dc_id = 0;
            rep.rv = 0;
            return rep;
        }

        // need to check, if last parent is DC
        db_reply <db_web_basic_element_t> ret = select_asset_element_web_byId
                                            (conn, dc_id);
        if ( ret.status == 1 )
        {
            if ( ret.item.type_name == "datacenter" )
            {
                rep.rv = 0;
                LOG_END;
                return rep;
            }
            else
            {
                rep.rv = 2;
                log_debug ("last parent is not a datacenter");
                return rep;
            }
        }
        else
        {
            rep.rv = ret.errsubtype;
            dc_id = 0;
            log_debug ("could not check the type of last element");
            return rep;
        }
    }
    catch (const tntdb::NotFound &e) {
        rep.rv = 3;
        dc_id = 0;
        LOG_END_ABNORMAL(e);
        return rep;
    }
    catch (const std::exception &e) {
        rep.rv = -1;
        dc_id = 0;
        LOG_END_ABNORMAL(e);
        return rep;
    }
    catch (...) {
        log_error ("Uncknown exception caught!");
        rep.rv = -2;
        dc_id = 0;
        return rep;
    }
}

int
    select_asset_element_all(
            tntdb::Connection& conn,
            std::function<void(
                const tntdb::Row&
                )>& cb)
{
    LOG_START;

    try{
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "   v.id, v.name, v.type_name,"
            "   v.subtype_name, v.id_parent,"
            "   v.business_crit, v.status, v.priority,"
            "   v.asset_tag"
            " FROM"
            "   v_web_element v"
        );

        tntdb::Result res = st.select();

        for (const auto& r: res) {
            cb(r);
        }
        LOG_END;
        return 0;
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return -1;
    }
}

int
    select_ext_attributes_keytags(
            tntdb::Connection& conn,
            std::function<void(
                const tntdb::Row&
                )>& cb)
{
    LOG_START;
    try{
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "   DISTINCT(keytag)"
            " FROM"
            "   v_bios_asset_ext_attributes"
        );

        tntdb::Result res = st.select();

        for (const auto& r: res) {
            cb(r);
        }
        LOG_END;
        return 0;
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return -1;
    }
}

int
select_group_names(
        tntdb::Connection& conn,
        a_elmnt_id_t id,
        std::function<void(const tntdb::Row&)> cb)
{
    LOG_START;
    log_debug("id: %" PRIu32, id);
    try{
        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            "   v2.name "
            " FROM v_bios_asset_group_relation v1 "
            " JOIN v_bios_asset_element v2 "
            "   ON v1.id_asset_group=v2.id "
            " WHERE v1.id_asset_element=:id "
        );

        tntdb::Result res = st.set("id", id).select();

        for (const auto& r: res) {
            cb(r);
        }
        LOG_END;
        return 0;
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return -1;
    }
}

int
select_group_names(
        tntdb::Connection& conn,
        a_elmnt_id_t id,
        std::vector<std::string>& out)
{
    std::function<void(const tntdb::Row&)> func = \
        [&out](const tntdb::Row& r)
        {
            std::string name;
            r["name"].get(name);
            out.push_back(name);
        };
    return select_group_names(conn, id, func);
}

int
select_v_web_asset_power_link_src_byId(
        tntdb::Connection& conn,
        a_elmnt_id_t id,
        row_cb_f& cb)
{
    LOG_START;
    log_debug("id: %" PRIu32, id);
    try{
        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            "   v.id_link, "
            "   v.id_asset_element_src, "
            "   v.src_name, "
            "   v.id_asset_element_dest, "
            "   v.dest_name, "
            "   v.src_out, "
            "   v.dest_in "
            " FROM v_web_asset_link v "
            " WHERE v.id_asset_element_dest=:id "
            " AND v.link_name = 'power chain' "
        );

        tntdb::Result res = st.set("id", id).select();

        for (const auto& r: res) {
            cb(r);
        }
        LOG_END;
        return 0;
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return -1;
    }
}

int
max_number_of_asset_groups(
        tntdb::Connection& conn)
{
    LOG_START;

    try{
        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            "   MAX(grp_count) "
            " FROM "
            "   ( SELECT COUNT(*) grp_count FROM t_bios_asset_group_relation "
            "            GROUP BY id_asset_element) elmnt_grp "
        );

        tntdb::Row row = st.selectRow();

        int r = 0;
        row[0].get(r);
        LOG_END;
        return r;
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return -1;
    }
}

int
max_number_of_power_links(
        tntdb::Connection& conn)
{
    LOG_START;

    try{
        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            "   MAX(power_src_count) "
            " FROM "
            "   ( SELECT COUNT(*) power_src_count FROM t_bios_asset_link "
            "            GROUP BY id_asset_device_dest) pwr "
        );

        tntdb::Row row = st.selectRow();

        int r = 0;
        row[0].get(r);
        LOG_END;
        return r;
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return -1;
    }
}

int
count_of_link_src(
        tntdb::Connection& conn,
        a_elmnt_id_t id)
{
    LOG_START;
    static const int id_asset_link_type = 1;
    try{
        tntdb::Statement st = conn.prepareCached(
            " SELECT COUNT( * ) "
            " FROM v_bios_asset_link "
            " WHERE id_asset_element_src = :id AND"
            "       id_asset_link_type = :lt "
        );

        tntdb::Row row = st.\
            set("id", id).\
            set("lt", id_asset_link_type).\
            selectRow();

        int r = 0;
        row[0].get(r);
        LOG_END;
        return r;
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return -1;
    }
}

int
unique_keytag(
        tntdb::Connection &conn,
        const std::string &keytag,
        const std::string &value,
        a_elmnt_id_t       element_id)
{
    LOG_START;

    try{
        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            "   id_asset_element "
            " FROM "
            "   t_bios_asset_ext_attributes "
            " WHERE keytag = :keytag AND"
            "       value = :value"
        );

        tntdb::Row row = st.set("keytag", keytag)
                           .set("value", value)
                           .selectRow();

        a_elmnt_id_t r = 0;
        row[0].get(r);

        LOG_END;
        if ( element_id == r )
            return 0; // is ok
        else
            return 1; // is not ok
    }
    catch (const tntdb::NotFound &e) {
        LOG_END_ABNORMAL(e);
        return 0; // ok
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return -1;
    }
}

db_reply_t
    select_monitor_device_type_id
        (tntdb::Connection &conn,
         const char *device_type_name)
{
    LOG_START;

    log_debug ("  device_type_name = %s", device_type_name);

    db_reply_t ret = db_reply_new();

    try{
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "   v.id"
            " FROM"
            "   v_bios_device_type v"
            " WHERE v.name = :name"
        );

        tntdb::Value val = st.set("name", device_type_name).
                              selectValue();
        log_debug ("[t_bios_monitor_device]: was selected 1 rows");

        val.get(ret.item);
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const tntdb::NotFound &e){
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_NOTFOUND;
        ret.msg        = e.what();
        log_info ("end: discovered device type was not found with '%s'", e.what());
        return ret;
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL (e);
        return ret;
    }
}

int
    convert_asset_to_monitor(
        tntdb::Connection &conn,
        a_elmnt_id_t       asset_element_id,
        m_dvc_id_t        &monitor_element_id)
{
    try{
        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            "   v.id_discovered_device "
            " FROM "
            "   v_bios_monitor_asset_relation v "
            " WHERE "
            "   v.id_asset_element = :id "
        );

        tntdb::Value value = st.set("id", asset_element_id).
                                selectValue();

        value.get(monitor_element_id);
        LOG_END;
        return 0;
    }
    catch (const tntdb::NotFound &e){
        log_info("end: counterpart for %" PRIu32 " notfound", asset_element_id);
        monitor_element_id = 0;
        return 0;
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return -1;
    }
}

int
    select_assets_by_container
        (tntdb::Connection &conn,
         a_elmnt_id_t element_id,
         std::function<void(const tntdb::Row&)>& cb
         )
{
    LOG_START;
    log_debug ("container element_id = %" PRIu32, element_id);

    try {
        // Can return more than one row.
        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            "   v.name, "
            "   v.id_asset_element as asset_id, "
            "   v.id_asset_device_type as subtype_id, "
            "   v.type_name as subtype_name, "
            "   v.id_type as type_id "
            " FROM "
            "   v_bios_asset_element_super_parent v "
            " WHERE "
            "   :containerid in (v.id_parent1, v.id_parent2, v.id_parent3, v.id_parent4, v.id_parent5)"
        );

        tntdb::Result result = st.set("containerid", element_id).
                                  select();
        log_debug("[v_bios_asset_element_super_parent]: were selected %" PRIu32 " rows",
                                                            result.size());
        for ( auto &row: result ) {
            cb(row);
        }
        LOG_END;
        return 0;
    }
    catch (const std::exception& e) {
        LOG_END_ABNORMAL(e);
        return -1;
    }
}

int select_asset_ext_attribute_by_keytag(
    tntdb::Connection &conn,
    const std::string &keytag,
    const std::set<a_elmnt_id_t> &element_ids,
    std::function< void( const tntdb::Row& ) > &cb)
{
    LOG_START;
    try{
        std::string inlist;
        for( const auto &id : element_ids ) {
            inlist += ",";
            inlist += std::to_string(id);
        }
        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            "   id_asset_ext_attribute, keytag, value, "
            "   id_asset_element, read_only "
            " FROM "
            "   v_bios_asset_ext_attributes "
            " WHERE keytag = :keytag" +
            ( element_ids.empty() ? "" : " AND id_asset_element in (" + inlist.substr(1) + ")" )
        );
        tntdb::Result rows = st.set("keytag", keytag ).select();
        for( const auto &row: rows ) cb( row );
        LOG_END;
        return 0;
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return -1;
    }
}

} // namespace end
