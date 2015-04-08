/*
Copyright (C) 2014-2015 Eaton
 
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

/*! \file assetcr.cc
    \brief Pure DB API for insert for different tables

    \author Alena Chernikava <alenachernikava@eaton.com>
*/

#include <tntdb/connect.h>
#include <tntdb/row.h>
#include <tntdb/result.h>
#include <tntdb/error.h>
#include <tntdb/transaction.h>

#include "log.h"
#include "defs.h"
#include "assetcrud.h"

#include "monitor.h" // is_ok_... functions

//////////////////////////////////////////////////////////////////////////////
///                               INSERT API                  ////////////////
//////////////////////////////////////////////////////////////////////////////

//=============================================================================
db_reply_t
    insert_asset_element_into_asset_group 
        (tntdb::Connection &conn,
         a_elmnt_id_t group_id,
         a_elmnt_id_t asset_element_id)
{
    LOG_START;
    log_debug ("  group_id = %" PRIu32, group_id);
    log_debug ("  asset_element_id = %" PRIu32, asset_element_id);

    db_reply_t ret = db_reply_new();

    // input parameters control 
    if ( asset_element_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of asset_element_id is not allowed";
        log_error ("end: %s, %s", "ignore insert", ret.msg);
        return ret;
    }
    if ( group_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of group_id is not allowed";
        log_error ("end: %s, %s","ignore insert", ret.msg);
        return ret;
    }
    log_debug ("input parameters are correct");
    
    try{
        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            "   t_bios_asset_group_relation"
            "   (id_asset_group, id_asset_element)"
            " SELECT"
            "   :group, :element"
            " FROM"
            "   t_empty"
            " WHERE NOT EXISTS"
            "   ("
            "       SELECT"
            "           id_asset_group"
            "       FROM"
            "           t_bios_asset_group_relation"
            "       WHERE"
            "           id_asset_group = :group AND"
            "           id_asset_element = :element"
            "   )"
        );
   
        ret.affected_rows = st.set("group"  , group_id).
                               set("element", asset_element_id).
                               execute();
        ret.rowid = conn.lastInsertId();
        log_debug ("[t_bios_asset_group_relation]: was inserted %" 
                                    PRIu64 " rows", ret.affected_rows);
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
db_reply_t
    insert_into_asset_device
        (tntdb::Connection &conn,
         a_elmnt_id_t   asset_element_id,
         a_dvc_tp_id_t  asset_device_type_id)
{
    LOG_START;
    log_debug ("  asset_element_id = %" PRIu32, asset_element_id);
    log_debug ("  asset_device_type_id = %" PRIu32, asset_device_type_id);

    db_reply_t ret = db_reply_new();

    // input parameters control 
    if ( asset_element_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of asset_element_id is not allowed";
        log_error ("end: %s, %s", "ignore insert", ret.msg);
        return ret;
    }
    if ( !is_ok_asset_device_type (asset_device_type_id) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of asset_device_type_id is not allowed";
        log_error ("end: %s, %s", "ignore insert", ret.msg);
        return ret;
    }
    log_debug ("input parameters are correct");

    try{
        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            "   t_bios_asset_device"
            "   (id_asset_element, id_asset_device_type)"
            " SELECT"
            "   :element, :type"
            " FROM"
            "   t_empty"
            " WHERE NOT EXISTS"
            "   ("
            "       SELECT"
            "           id_asset_element"
            "       FROM"
            "           t_bios_asset_device"
            "       WHERE"
            "           id_asset_element = :element"
            "   )"
        );
   
        ret.affected_rows = st.set("element", asset_element_id).
                               set("type", asset_device_type_id).
                               execute();
        ret.rowid = conn.lastInsertId();
        log_debug ("[t_bios_asset_device]: was inserted %" 
                                        PRIu64 " rows", ret.affected_rows);
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
// TODO: check, if it works with multiple powerlinks between two devices
db_reply_t
    insert_into_asset_link
        (tntdb::Connection &conn,
         a_elmnt_id_t    asset_element_src_id,
         a_elmnt_id_t    asset_element_dest_id,
         a_lnk_tp_id_t   link_type_id,
         const a_lnk_src_out_t src_out,
         const a_lnk_dest_in_t dest_in)
{
    LOG_START;
    log_debug ("  asset_element_src_id = %" PRIu32, asset_element_src_id);
    log_debug ("  asset_element_dest_id = %" PRIu32, asset_element_dest_id);
    log_debug ("  link_type_id = %" PRIu32, link_type_id);
    log_debug ("  src_out = '%s'", src_out);
    log_debug ("  dest_in = '%s'", dest_in);
    
    db_reply_t ret = db_reply_new();
    
    // input parameters control 
    if ( asset_element_dest_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "destination device is not specified";
        log_error ("end: %s, %s", "ignore insert", ret.msg);
        return ret;
    }
    if ( asset_element_src_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "source device is not specified";
        log_error ("end: %s, %s","ignore insert", ret.msg);
        return ret;
    }
    if ( !is_ok_link_type (link_type_id) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "wrong link type";
        log_error ("end: %s, %s","ignore insert", ret.msg);
        return ret;
    }
    // src_out and dest_in can take any value from available range
    log_debug ("input parameters are correct");

    try{
        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            "   t_bios_asset_link"
            "   (id_asset_device_src, id_asset_device_dest,"
            "        id_asset_link_type, src_out, dest_in)"
            " SELECT"
            "   v1.id_asset_device, v2.id_asset_device, :linktype,"
            "   :out, :in"
            " FROM"
            "   t_bios_asset_device v1,"  // src
            "   t_bios_asset_device v2"   // dvc
            " WHERE"
            "   v1.id_asset_element = :src AND"
            "   v2.id_asset_element = :dest AND"
            "   NOT EXISTS"
            "     ("
            "           SELECT"
            "             id_link"
            "           FROM"
            "             t_bios_asset_link v3"
            "           WHERE"
            "               v3.id_asset_device_src = v1.id_asset_device AND"
            "               v3.id_asset_device_dest = v2.id_asset_device AND"
            "               ( ((v3.src_out == :out) AND (v3.dest_in == :in)) OR ( v3.src_out is NULL) OR (v3.dest_in is NULL) ) "
            "               v3.id_asset_device_dest = v2.id_asset_device AND"
            "    )"
        );
        
        if ( !strcmp(src_out, "") )
            st.setNull("out");
        else
            st.set("out", src_out);
        if ( !strcmp(dest_in, "") )
            st.setNull("in");
        else
            st.set("in", dest_in);

        ret.affected_rows = st.set("src", asset_element_src_id).
                               set("dest", asset_element_dest_id).
                               set("linktype", link_type_id).
                               execute();
        ret.rowid = conn.lastInsertId();
        log_debug ("[t_bios_asset_device]: was inserted %" 
                                        PRIu64 " rows", ret.affected_rows);
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
db_reply_t
    insert_into_asset_ext_attribute
        (tntdb::Connection &conn,
         const char   *value,
         const char   *keytag,
         a_elmnt_id_t  asset_element_id)
{
    LOG_START;
    log_debug ("  value = '%s'", value);
    log_debug ("  keytag = %s", keytag);
    log_debug ("  asset_element_id = %" PRIu32, asset_element_id);

    db_reply_t ret = db_reply_new();

    // TODO: RACK: ("type", "4"));
    // should we check this parameter for range of available values
    // input parameters control 
    if ( asset_element_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of asset_element_id is not allowed";
        log_error ("end: %s, %s", "ignore insert", ret.msg);
        return ret;
    }
    if ( !is_ok_value (value) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "unexpected value of value";
        log_error ("end: %s, %s", "ignore insert", ret.msg);
        return ret;
    }
    if ( !is_ok_keytag (keytag) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "unexpected value of keytag";
        log_error ("end: %s, %s", "ignore insert", ret.msg);
        return ret;
    }
    log_debug ("input parameters are correct");

    try{
        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            "   t_bios_asset_ext_attributes"
            "   (keytag, value, id_asset_element)"
            " SELECT"
            "   :keytag, :value, :element"
            " FROM"
            "   t_empty"
            " WHERE NOT EXISTS"
            "   ("
            "       SELECT"
            "           id_asset_element"
            "       FROM"
            "           t_bios_asset_ext_attributes"
            "       WHERE"
            "           keytag = :keytag AND"
            "           id_asset_element = :element"
            "   )"
        );
   
        ret.affected_rows = st.set("keytag" , keytag).
                               set("value"  , value).
                               set("element", asset_element_id).
                               execute();
        ret.rowid = conn.lastInsertId();
        log_debug ("[t_bios_asset_ext_attributes]: was inserted %" 
                                        PRIu64 " rows", ret.affected_rows);
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

// hash is untouched
//=============================================================================
db_reply_t
    insert_into_asset_ext_attributes
        (tntdb::Connection &conn, 
         zhash_t      *attributes,
         a_elmnt_id_t  asset_element_id)
{
    LOG_START;
    log_debug ("  asset_element_id = %" PRIu32, asset_element_id);
    
    db_reply_t ret = db_reply_new();

    if ( asset_element_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of asset_element_id is not allowed";
        log_error ("end: %s, %s", "ignore insert", ret.msg);
        return ret;
    }
    if ( ( attributes == NULL ) || ( zhash_size (attributes) == 0 ) )
    {
        log_debug ("nothing to insert");
        ret.status = 1;
        LOG_END;
        // actually, if there is nothing to insert, then insert was ok :)
        return ret;
    }
    log_debug ("input parameters are correct");

    char *value = (char *) zhash_first (attributes);   // first value
    
    // there is no support of bulk operations, 
    // so if there is more than one ext 
    // atrtribute we will insert them all iteratevely
    // the hash "attributes" is a finite set, so the cycle will 
    // end in finite number of steps
    while ( value != NULL )
    {
        char *key = (char *) zhash_cursor (attributes); // key of this value
        auto reply_internal = insert_into_asset_ext_attribute 
                                    (conn, value, key, asset_element_id);
        if ( reply_internal.status == 1 )
            ret.affected_rows++;
        value = (char *) zhash_next (attributes);       // next value
    }

    if ( ret.affected_rows == zhash_size (attributes) )
    {
        ret.status = 1;
        log_debug ("all ext attributes were inserted successfully");
        LOG_END;
        return ret;
    }
    else
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "not all attributes were inserted";
        log_error ("end: %s", ret.msg);
        return ret;
    }
}

//=============================================================================
db_reply_t
    insert_into_asset_element
        (tntdb::Connection &conn, 
         const char      *element_name, 
         a_elmnt_tp_id_t  element_type_id,
         a_elmnt_id_t     parent_id,
         const char      *status,
         a_elmnt_pr_t     priority,
         a_elmnt_bc_t     bc)
{
    LOG_START;
    log_debug ("  element_name = '%s'", element_name);
    log_debug ("  element_type_id = %" PRIu32, element_type_id);
    log_debug ("  parent_id = %" PRIu32, parent_id);
    log_debug ("  status = '%s'", status);
    log_debug ("  priority = %" PRIu16, priority);
    log_debug ("  bc = %" PRIu16, bc);

    db_reply_t ret = db_reply_new();
 
    // input parameters control 
    if ( !is_ok_name_length (element_name) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "name is not valid";
        log_error ("end: %s, %s", "ignore insert", ret.msg);
        return ret;
    }
    if ( !is_ok_element_type (element_type_id) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of element_type_id is not allowed";
        log_error ("end: %s, %s", "ignore insert", ret.msg);
        return ret;
    }
    // ASSUMPTION: all datacenters are unlockated elements
    if ( ( element_type_id == asset_type::DATACENTER ) && 
         ( parent_id != 0 ) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "Datacenters should be unlockated elements";
        log_error ("end: %s, %s", "ignore insert", ret.msg);
        return ret;
    }
    // TODO:should we add more checks here???
    log_debug ("input parameters are correct");
    
    try{
        tntdb::Statement st;
        if ( parent_id == 0 )
        {
            st = conn.prepareCached(
                " INSERT INTO"
                "   t_bios_asset_element"
                "   (name, id_type, id_parent, status, priority, business_crit)"
                " SELECT"
                "   :name, :type, NULL, :status, :priority, :business_crit"
                " FROM"
                "   t_empty"
                " WHERE NOT EXISTS"
                "   ("
                "       SELECT"
                "         id_type"
                "       FROM"
                "         t_bios_asset_element"
                "       WHERE"
                "         name = :name AND"
                "         id_type = :type AND"
                "         id_parent is NULL" 
                "   )"
            );

            ret.affected_rows = st.set("name", element_name).
                                   set("type", element_type_id).
                                   set("status", status).
                                   set("priority", priority).
                                   set("business_crit", bc).
                                   execute();
        }
        else
        {
            st = conn.prepareCached(
                " INSERT INTO"
                "   t_bios_asset_element"
                "   (name, id_type, id_parent)"
                " SELECT"
                "   :name, :type, :parent"
                " FROM"
                "   t_empty"
                " WHERE NOT EXISTS"
                "   ("
                "       SELECT"
                "         id_type"
                "       FROM"
                "         t_bios_asset_element"
                "       WHERE"
                "         name = :name AND"
                "         id_type = :type AND"
                "         id_parent = :parent" 
                "   )"
            );
            ret.affected_rows = st.set("name", element_name).
                                   set("type", element_type_id).
                                   set("parent", parent_id).
                                   set("status", status).
                                   set("priority", priority).
                                   set("business_crit", bc).
                                   execute();
        }
        ret.rowid = conn.lastInsertId();
        log_debug ("[t_bios_asset_element]: was inserted %" 
                                        PRIu64 " rows", ret.affected_rows);
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
db_reply_t
    insert_into_asset_links
        (tntdb::Connection       &conn,
         std::vector <link_t> const &links)
{
    LOG_START;
    
    db_reply_t ret = db_reply_new();
    
    // input parameters control 
    if ( links.size() == 0 )
    {
        log_debug ("nothing to insert");
        ret.status = 1;
        LOG_END;
        // actually, if there is nothing to insert, then insert was ok :)
        return ret;
    }
    log_debug ("input parameters are correct");

    for ( auto &one_link : links )
    {
        auto ret_internal = insert_into_asset_link
                (conn, one_link.src, one_link.dest, one_link.type,
                       one_link.src_out, one_link.dest_in);
        if ( ret_internal.status == 1 )
            ret.affected_rows++;
    }
    if ( ret.affected_rows == links.size() )
    {
        ret.status = 1;
        log_debug ("all linnks were inserted successfully");
        LOG_END;
        return ret;
    }
    else
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "not all links were inserted";
        log_error ("end: %s", ret.msg);
        return ret;
    }
}

//=============================================================================
static std::string
    grp_values
        (std::set <a_elmnt_id_t> const &groups,
         a_elmnt_id_t                   asset_element_id)
{
    std::string result = "  ";
    for ( auto &grp : groups )
    {
        result = result + "(" + std::to_string(grp) + 
                          "," + std::to_string(asset_element_id) + ")" + ",";
    }
    // need to remove last ","
    result.back() = ' ';
    return result;
}

db_reply_t
    insert_element_into_groups
        (tntdb::Connection &conn, 
         std::set <a_elmnt_id_t> const &groups,
         a_elmnt_id_t                   asset_element_id)
{
    LOG_START;
    log_debug ("  asset_element_id = %" PRIu32, asset_element_id);

    db_reply_t ret = db_reply_new();

    // input parameters control 
    if ( asset_element_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of asset_element_id is not allowed";
        log_error ("end: %s, %s", "ignore insert", ret.msg);
        return ret;
    }
    if ( groups.size() == 0 )
    {
        log_debug ("nothing to insert");
        ret.status = 1;
        LOG_END;
        // actually, if there is nothing to insert, then insert was ok :)
        return ret;
    }
    log_debug ("input parameters are correct");

    try{
        tntdb::Statement st = conn.prepare(
            " INSERT INTO"
            "   t_bios_asset_group_relation"
            "   (id_asset_group, id_asset_element)"
            " VALUES " + grp_values(groups, asset_element_id)
        );
        
        ret.affected_rows = st.execute();
        log_debug ("[t_bios_asset_group_relation]: was inserted %" 
                                PRIu64 " rows", ret.affected_rows);
        
        if ( ret.affected_rows == groups.size() )
        {
            ret.status = 1;
            log_debug ("all links were inserted successfully");
            LOG_END;
        }
        else
        {
            ret.status     = 0;
            ret.errtype    = DB_ERR;
            ret.errsubtype = DB_ERROR_BADINPUT;
            ret.msg        = "not all links were inserted";
            log_error ("end: %s", ret.msg);
        }
        return ret;
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
// transaction is used
db_reply_t
    insert_dc_room_row_rack_group
    (tntdb::Connection  &conn,
     const char      *element_name,
     a_elmnt_tp_id_t  element_type_id,
     a_elmnt_id_t     parent_id,
     zhash_t         *extattributes,
     const char      *status,
     a_elmnt_pr_t     priority,
     a_elmnt_bc_t     bc)
{
    LOG_START;
    log_debug ("  element_name = '%s'", element_name);
    log_debug ("  element_type_id = %" PRIu32, element_type_id);
    log_debug ("  parent_id = %" PRIu32, parent_id);

    tntdb::Transaction trans(conn);

    auto reply_insert1 = insert_into_asset_element
                        (conn, element_name, element_type_id, parent_id, status, priority, bc);
    if ( reply_insert1.affected_rows == 0 )
    {
        trans.rollback();
        log_error ("end: element was not inserted");
        return reply_insert1;
    }
    auto element_id = reply_insert1.rowid;

    auto reply_insert2 = insert_into_asset_ext_attributes
                         (conn, extattributes, element_id);
    if ( reply_insert2.affected_rows == 0 )
    {
        trans.rollback();
        log_error ("end: element was not inserted (fail in ext_attributes)");
        return reply_insert2;
    }

    trans.commit();
    LOG_END;
    return reply_insert1;
}

// because of transactions, previos function is not used here!
// u links, neni definovan dest, prortoze to jeste neni znamo, tak musime
// uvnitr funkce to opravit
db_reply_t
    insert_device
       (tntdb::Connection &conn,
        std::vector <link_t> &links,
        std::set <a_elmnt_id_t> const &groups,
        const char    *element_name, 
        a_elmnt_id_t   parent_id,
        zhash_t       *extattributes,
        a_dvc_tp_id_t  asset_device_type_id,
        const char    *status,
        a_elmnt_pr_t   priority,
        a_elmnt_bc_t   bc)
{
    LOG_START;
    log_debug ("  element_name = '%s'", element_name);
    log_debug ("  parent_id = %" PRIu32, parent_id);
    log_debug ("  asset_device_type_id = %" PRIu32, asset_device_type_id);

    tntdb::Transaction trans(conn);
        
    auto reply_insert1 = insert_into_asset_element
                        (conn, element_name, asset_type::DEVICE, parent_id, status, priority, bc);
    if ( reply_insert1.affected_rows == 0 )
    {
        trans.rollback();
        log_info ("end: device was not inserted (fail in element)");
        return reply_insert1;
    }
    auto element_id = reply_insert1.rowid;

    auto reply_insert2 = insert_into_asset_ext_attributes
                        (conn, extattributes, element_id);
    if ( reply_insert2.affected_rows == 0 )
    {
        trans.rollback();
        log_error ("end: device was not inserted (fail in ext_attributes)");
        return reply_insert2;
    }
           
    auto reply_insert3 = insert_element_into_groups (conn, groups, element_id);
    if ( reply_insert3.affected_rows == 0 )
    {
        trans.rollback();
        log_info ("end: device was not inserted (fail into groups)");
        return reply_insert3;
    }
    
    auto reply_insert4 = insert_into_asset_device
        (conn, element_id, asset_device_type_id);
    if ( reply_insert4.affected_rows == 0 )
    {
        trans.rollback();
        log_info ("end: device was not inserted (fail asset_device)");
        return reply_insert4;
    }

    for ( auto &one_link: links )
    {
        one_link.src = element_id;
    }

    auto reply_insert5 = insert_into_asset_links
           (conn, links);
    if ( reply_insert5.affected_rows != links.size() )
    {
        trans.rollback();
        log_info ("end: not all links were inserted (fail asset_link)");
        return reply_insert5;
    }

    trans.commit();
    LOG_END;
    return reply_insert1;
}
