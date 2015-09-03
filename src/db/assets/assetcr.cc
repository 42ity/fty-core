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

/*! \file   assetcr.cc
    \brief  Pure DB API for insert for different tables
    \author Alena Chernikava <AlenaChernikava@Eaton.com>
*/
#include "assetcr.h"

#include <tntdb/row.h>
#include <tntdb/result.h>
#include <tntdb/error.h>
#include <tntdb/transaction.h>

#include "log.h"
#include "defs.h"
#include "asset_types.h"

namespace persist {

static const std::string  ins_upd_ass_ext_att_QUERY =
        " INSERT INTO"
        "   t_bios_asset_ext_attributes"
        "   (keytag, value, id_asset_element, read_only)"
        " VALUES"
        "  ( :keytag, :value, :element, :readonly)"
        " ON DUPLICATE KEY"
        "   UPDATE"
        "       value = VALUES (value),"
        "       read_only = 1,"
        "       id_asset_ext_attribute = LAST_INSERT_ID(id_asset_ext_attribute)";
// update doesnt return id of updated row -> use workaround

static const std::string  ins_ass_ext_att_QUERY =
        " INSERT INTO"
        "   t_bios_asset_ext_attributes"
        "   (keytag, value, id_asset_element, read_only)"
        " SELECT"
        "   :keytag, :value, :element, :readonly"
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
        "   )";

static db_reply_t
    insert_into_asset_ext_attribute_template
        (tntdb::Connection &conn,
         const char   *value,
         const char   *keytag,
         a_elmnt_id_t  asset_element_id,
         bool          read_only,
         std::string   query)
{
    LOG_START;

    log_debug ("value = '%s'", value);
    log_debug ("keytag = '%s'", keytag);
    log_debug ("asset_element_id = %" PRIu32, asset_element_id);
    log_debug ("read_only = %d", read_only);

    a_ext_attr_id_t newid = 0;
    a_ext_attr_id_t n     = 0; // number of rows affected

    db_reply_t ret = db_reply_new();
    // input parameters control
    if ( asset_element_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "apropriate asset element is not specified";
        log_error ("end: ignore insert, apropriate asset element is "
                                                         "not specified");
        return ret;
    }
    if ( !is_ok_value (value) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "unexepetable value";
        log_error ("end: ignore insert, unexeptable value '%s'", value);
        return ret;
    }
    if ( !is_ok_keytag (keytag) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "unexepetable keytag";
        log_error ("end: ignore insert, unexeptable keytag '%s'", keytag);
        return ret;
    }
    log_debug ("input parameters are correct");

    try {

        tntdb::Statement st = conn.prepareCached(query);

        n = st.set("keytag"  , keytag).
               set("value"   , value).
               set("readonly", read_only).
               set("element" , asset_element_id).
               execute();
        newid = conn.lastInsertId();
        log_debug ("was inserted %" PRIu32 " rows", n);
        ret.affected_rows = n;
        ret.rowid = newid;
        // attention:
        //  -- 0 rows can be inserted
        //        - there is no free space
        //        - FK on id_asset_element
        //        - row is already inserted
        //        - in some other, but not normal cases
        //  -- 1 row is inserted - a usual case
        //  -- more than one row, it is not normal and it is not expected
        //       due to nature of the insert statement
    }
    catch (const std::exception &e) {
        ret.affected_rows = n;
        ret.status     = 0;
        ret.errtype       = DB_ERR;
        ret.errsubtype    = DB_ERROR_INTERNAL;
        ret.msg           = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
    // a statement "insert on duplicate update
    // return 2 affected rows when update is used and updated value was different from previos
    // return 0 affected rows when update is used and updated value is the same as previos
    if ( ( n == 1 ) ||
         ( ( ( n == 2 ) || ( n == 0 ) )&& ( read_only) ) )
    {
        ret.status = 1;
        LOG_END;
    }
    else
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "unexpected number of returned rows";
        log_info ("end: %" PRIu32 " - unexpected number of rows returned", n);
    }
    return ret;
}


db_reply_t
    insert_into_asset_ext_attribute
        (tntdb::Connection &conn,
         const char   *value,
         const char   *keytag,
         a_elmnt_id_t  asset_element_id,
         bool          read_only)
{
    if ( !read_only )
    {
        log_debug ("use pure insert");
        return insert_into_asset_ext_attribute_template
            (conn, value, keytag, asset_element_id, read_only,
             ins_ass_ext_att_QUERY);
    }
    else
    {
        log_debug ("use insert on duplicate update");
        return insert_into_asset_ext_attribute_template
            (conn, value, keytag, asset_element_id, read_only,
             ins_upd_ass_ext_att_QUERY);
    }
}


// hash left untouched
db_reply_t
    insert_into_asset_ext_attributes
        (tntdb::Connection &conn,
         zhash_t      *attributes,
         a_elmnt_id_t  asset_element_id,
         bool          read_only)
{
    LOG_START;

    m_msrmnt_id_t n = 0; // number of rows affected
    db_reply_t ret = db_reply_new();

    // input parameters control
    if ( asset_element_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "apropriate asset element is not specified";
        log_error ("end: ignore insert, apropriate asset element is "
                                                         "not specified");
        return ret;
    }
    if ( attributes == NULL )
    {
        ret.status     = 1;
        log_info ("end: ignore insert, ext attributes are "
                                                    "not specified (NULL)");
        return ret;
    }
    if ( zhash_size (attributes) == 0 )
    {
        ret.status     = 1;
        log_info ("end: nothing to insert");
        // actually, if there is nothing to insert, then insert was ok :)
        // but we need to return an id, so the only available non valid
        // value is zero.
        return ret;
    }
    log_debug ("input parameters are correct");

    char *value = (char *) zhash_first (attributes);   // first value

    // there is no supported bulk operations,
    // so if there is more than one ext
    // atrtribute we will insert them all iteratevely
    // the hash "attributes" is a finite set, so the cycle will
    // end in finite number of steps

    // it possible to generate insert as "insert into table values (),(),();" But here it
    // can cause a secuire problems, because SQL injection can be abused here,
    // bcause keytag and value are unknown strings
    while ( value != NULL )
    {
        char *key = (char *) zhash_cursor (attributes);   // key of this value
        ret       = insert_into_asset_ext_attribute (conn, value, key, asset_element_id, read_only);
        if ( ret.status == 1 )
            n++;
        value     = (char *) zhash_next (attributes);   // next value
    }
    ret.affected_rows = n;
    if ( n == zhash_size (attributes) )
        LOG_END;
    else
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "not all ext attributes were inserted";
        log_error ("end: not all ext attributes were inserted");
    }
    return ret;
}



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
        log_error ("end: %s, %s", "ignore insert", ret.msg.c_str());
        return ret;
    }
    if ( group_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of group_id is not allowed";
        log_error ("end: %s, %s","ignore insert", ret.msg.c_str());
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
        log_error ("end: %s, %s", "ignore insert", ret.msg.c_str());
        return ret;
    }
    if ( asset_element_src_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "source device is not specified";
        log_error ("end: %s, %s","ignore insert", ret.msg.c_str());
        return ret;
    }
    if ( !is_ok_link_type (link_type_id) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "wrong link type";
        log_error ("end: %s, %s","ignore insert", ret.msg.c_str());
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
            "   v1.id_asset_element, v2.id_asset_element, :linktype,"
            "   :out, :in"
            " FROM"
            "   v_bios_asset_device v1,"  // src
            "   v_bios_asset_device v2"   // dvc
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
            "               v3.id_asset_device_src = v1.id_asset_element AND"
            "               v3.id_asset_device_dest = v2.id_asset_element AND"
            "               ( ((v3.src_out = :out) AND (v3.dest_in = :in)) OR ( v3.src_out is NULL) OR (v3.dest_in is NULL) ) AND"
            "               v3.id_asset_device_dest = v2.id_asset_element"
            "    )"
        );

        if ( strcmp(src_out, "") == 0 )
            st = st.setNull("out");
        else
            st = st.set("out", src_out);

        if ( strcmp(dest_in, "") == 0)
            st = st.setNull("in");
        else
            st = st.set("in", dest_in);

        ret.affected_rows = st.set("src", asset_element_src_id).
                               set("dest", asset_element_dest_id).
                               set("linktype", link_type_id).
                               execute();

        ret.rowid = conn.lastInsertId();
        log_debug ("[t_bios_asset_link]: was inserted %"
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
/*
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
}*/

//=============================================================================
db_reply_t
    insert_into_asset_element
        (tntdb::Connection &conn,
         const char      *element_name,
         a_elmnt_tp_id_t  element_type_id,
         a_elmnt_id_t     parent_id,
         const char      *status,
         a_elmnt_pr_t     priority,
         a_elmnt_bc_t     bc,
         a_dvc_tp_id_t    subtype_id,
         const char      *asset_tag)
{
    LOG_START;
    log_debug ("  element_name = '%s'", element_name);
    log_debug ("  element_type_id = %" PRIu32, element_type_id);
    log_debug ("  parent_id = %" PRIu32, parent_id);
    log_debug ("  status = '%s'", status);
    log_debug ("  priority = %" PRIu16, priority);
    log_debug ("  bc = %" PRIu16, bc);
    log_debug ("  subtype_id = %" PRIu16, subtype_id);
    log_debug ("  asset_tag = %s", asset_tag);
    if ( subtype_id == 0 ) // use default
        subtype_id = 10;  // ATTENTION; need to be alligned with initdb

    db_reply_t ret = db_reply_new();

    // input parameters control
    if ( !is_ok_name (element_name) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "name is not valid";
        log_error ("end: %s, %s", "ignore insert", ret.msg.c_str());
        return ret;
    }
    if ( !is_ok_element_type (element_type_id) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of element_type_id is not allowed";
        log_error ("end: %s, %s", "ignore insert", ret.msg.c_str());
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
        log_error ("end: %s, %s", "ignore insert", ret.msg.c_str());
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
                "   (name, id_type, id_parent, status, priority,"
                "    business_crit, id_subtype, asset_tag)"
                " SELECT"
                "   :name, :type, NULL, :status, :priority,"
                "   :business_crit, :subtype, :assettag"
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
                                   set("subtype", subtype_id).
                                   set("assettag", asset_tag).
                                   execute();
        }
        else
        {
            st = conn.prepareCached(
                " INSERT INTO"
                "   t_bios_asset_element"
                "   (name, id_type, id_parent, status, priority,"
                "    business_crit, id_subtype, asset_tag)"
                " SELECT"
                "   :name, :type, :parent, :status, :priority,"
                "   :business_crit, :subtype, :assettag"
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
                                   set("subtype", subtype_id).
                                   set("assettag", asset_tag).
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
    if ( links.empty() )
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
        log_debug ("all links were inserted successfully");
        LOG_END;
        return ret;
    }
    else
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "not all links were inserted";
        log_error ("end: %s", ret.msg.c_str());
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
        log_error ("end: %s, %s", "ignore insert", ret.msg.c_str());
        return ret;
    }
    if ( groups.empty() )
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
            log_error ("end: %s", ret.msg.c_str());
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



db_reply_t
    insert_into_monitor_asset_relation
        (tntdb::Connection &conn,
         m_dvc_id_t   monitor_id,
         a_elmnt_id_t element_id)
{
    LOG_START;
    log_debug ("  monitor_id = %" PRIu32, monitor_id);
    log_debug ("  element_id = %" PRIu32, element_id);

    db_reply_t ret = db_reply_new();

    // input parameters control
    if ( element_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of element_id is not allowed";
        log_error ("end: %s, %s", "ignore insert", ret.msg.c_str());
        return ret;
    }
    if ( monitor_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of monitor_id is not allowed";
        log_error ("end: %s, %s", "ignore insert", ret.msg.c_str());
        return ret;
    }
    log_debug ("input parameters are correct");

    try{
        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            "   t_bios_monitor_asset_relation"
            "   (id_discovered_device, id_asset_element)"
            " VALUES"
            "   (:monitor, :asset)"
        );

        ret.affected_rows = st.set("monitor", monitor_id).
                               set("asset"  , element_id).
                               execute();
        ret.rowid = conn.lastInsertId();
        log_debug ("[t_bios_monitor_asset_relation]: was inserted %"
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

db_reply_t
    insert_into_monitor_device
        (tntdb::Connection &conn,
         m_dvc_tp_id_t device_type_id,
         const char* device_name)
{
    LOG_START;

    db_reply_t ret = db_reply_new();

    if ( !is_ok_name (device_name) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "device name length is not in range [1, MAX_NAME_LENGTH]";
        log_warning (ret.msg.c_str());
        return ret;
    }

    try{
        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            "   t_bios_discovered_device (name, id_device_type)"
            " VALUES (:name, :iddevicetype)"
            " ON DUPLICATE KEY"
            "   UPDATE"
            "       id_discovered_device = LAST_INSERT_ID(id_discovered_device)"
        );

        // Insert one row or nothing
        ret.affected_rows = st.set("name", device_name).
                               set("iddevicetype", device_type_id).
                               execute();
        log_debug ("[t_bios_discovered_device]: was inserted %" 
                                        PRIu64 " rows", ret.affected_rows);
        ret.rowid = conn.lastInsertId();
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


} // end namespace
