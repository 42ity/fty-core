/*
Copyright (C) 2014 Eaton
 
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

/*! \file assetcrud.cc
    \brief Basic functions for assets
    \author Alena Chernikava <alenachernikava@eaton.com>
*/

// ATTENTION: there is no easy way of getting last deleted id,
// and there is no requirements to do this.
// Then for every succesfull delete statement
// 0 would be return as rowid

#include <exception>
#include <assert.h>

#include <czmq.h>
#include <tntdb/connect.h>
#include <tntdb/row.h>
#include <tntdb/result.h>
#include <tntdb/error.h>
#include <tntdb/transaction.h>

#include "log.h"
#include "defs.h"
#include "dbpath.h"
#include "assetcrud.h"
#include "monitor.h"
#include "persist_error.h"
#include "asset_types.h"
#include "cleanup.h"

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


db_reply_t
    process_insert_inventory
        (tntdb::Connection &conn, const char *device_name, zhash_t *ext_attributes)
{
    LOG_START;
    tntdb::Transaction trans (conn);

    db_reply_t ret = select_device (conn, device_name);
    if ( ret.status == 0 )
        trans.commit(); // nothing was done, but we need to end the transaction
    else
    {
        m_dvc_id_t id = (m_dvc_id_t) ret.item;
        a_elmnt_id_t element_id = 0;
        // TODO get rid of oldstyle functions
        int rv = convert_monitor_to_asset_safe (url.c_str(), id, &element_id);
        if ( rv != 0 )
        {
            ret.errtype = DB_ERR;
            ret.errsubtype = DB_ERROR_BADINPUT; // anebo jiny kod???
            ret.msg = "";
            ret.status = 1;
            trans.commit(); // nothing was done, but we need to end the transaction
        }
        else
        {
            ret = insert_into_asset_ext_attributes (conn, ext_attributes, element_id, true);
            if ( ret.status == 0 )
            {
                ret.affected_rows = 0;
                trans.rollback();
            }
            else
                trans.commit();
        }
    }
    LOG_END;
    return ret;
}
static db_reply_t insert_into_asset_ext_attribute_template (tntdb::Connection &conn,
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
    insert_into_asset_ext_attribute (tntdb::Connection &conn,
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
db_reply_t insert_into_asset_ext_attributes (tntdb::Connection &conn, 
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
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "ext attributes are not specified (NULL)";
        log_error ("end: ignore insert, ext attributes are "
                                                    "not specified (NULL)");
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



zlist_t* select_asset_device_links_all(tntdb::Connection &conn,
                a_elmnt_id_t device_id, a_lnk_tp_id_t link_type_id)
{
    log_info("start");
    zlist_t* links = zlist_new();
    zlist_autofree(links);
    
    try {
        // Get information about the links the specified device 
        // belongs to
        // Can return more than one row
        
        tntdb::Result result;
        if ( link_type_id == 0 )
        {
            // take links of any type
            tntdb::Statement st_pow = conn.prepareCached(
                " SELECT"
                "   v.id_asset_element_dest, v.id_asset_element_src,"
                "   v.src_out, v.dest_in"
                " FROM"
                "   v_bios_asset_link v"
                " WHERE (v.id_asset_element_dest = :device OR"
                "       v.id_asset_element_src = :device)"
            );
            result = st_pow.set("device", device_id).
                            select();
        }
        else
        {
            // take links of specified type
            tntdb::Statement st_pow = conn.prepareCached(
                " SELECT"
                "   v.id_asset_element_dest, v.id_asset_element_src,"
                "   v.src_out, v.dest_in"
                " FROM"
                "   v_bios_asset_link v"
                " WHERE (v.id_asset_element_dest = :device OR"
                "       v.id_asset_element_src = :device) AND"
                "       v.id_asset_link_type = :link"
            );
            result = st_pow.set("device", device_id).
                            set("link", link_type_id).
                            select();
        }
        // TODO move 26 to constants
        char buff[26];     // 10+3+3+10

        // Go through the selected links
        for ( auto &row: result )
        { 
            // dest
            a_elmnt_id_t element_id_dest = 0;
            row[0].get(element_id_dest);
            assert ( element_id_dest != 0 );  // database is corrupted

            // src
            a_elmnt_id_t element_id_src = 0;
            row[1].get(element_id_src);
            assert ( element_id_src != 0 );  // database is corrupted

            // src_out
            std::string src_out = SRCOUT_DESTIN_IS_NULL;
            row[2].get(src_out);

            // dest_in
            std::string dest_in = SRCOUT_DESTIN_IS_NULL;
            row[3].get(dest_in);

            sprintf(buff, "%s:%" PRIu32 ":%s:%" PRIu32, 
                src_out.c_str(), 
                element_id_src, 
                dest_in.c_str(), 
                element_id_dest);
            zlist_push(links, buff);
        }
        log_info("end: normal");
        return links;
    }
    catch (const std::exception &e) {
        // internal error in database
        zlist_destroy (&links);
        log_warning("end: abnormal with '%s'", e.what());
        return NULL;
    }
}

std::set <a_elmnt_id_t> select_asset_group_elements (tntdb::Connection &conn, a_elmnt_id_t group_id)
{
    LOG_START;
    log_debug ("asset_group_id = %" PRIu32, group_id);
    
    try{
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "   v.id_asset_element"
            " FROM"
            "   t_bios_asset_group_relation v"
            " WHERE"
            "   v.id_asset_group = :group"
        );
    
        tntdb::Result result = st.set("group", group_id).
                                  select();

        log_debug ("was selected %u elements in group %" PRIu32, result.size(), group_id);
        
        std::set <a_elmnt_id_t> ret;
        a_elmnt_id_t asset_element_id = 0;
        for ( auto &row: result )
        {
            row[0].get(asset_element_id);
            ret.insert(asset_element_id);
        }

        return ret;
    }
    catch (const std::exception &e) {
        log_warning("end: abnormal with '%s'", e.what());
        throw bios::InternalDBError(e.what());
    }
}


/// get information from our database dictionaries
//
//

static std::string st_dictionary_element_type = \
            " SELECT"
            "   v.name, v.id"
            " FROM"
            "   v_bios_asset_element_type v";

static std::string st_dictionary_device_type = \
            " SELECT"
            "   v.name, v.id"
            " FROM"
            "   v_bios_asset_device_type v";

static 
db_reply < std::map <std::string, int> >
    get_dictionary
        (tntdb::Connection &conn, const std::string &st_str)
{
    LOG_START;
    std::map<std::string, int> mymap;
    db_reply < std::map<std::string, int> > ret = db_reply_new(mymap);

    try {
        tntdb::Statement st = conn.prepareCached(st_str);
        tntdb::Result res = st.select();
        
        std::string name = "";
        int id = 0;
        for ( auto &row : res )
        {
            row[0].get(name);
            row[1].get(id);
            ret.item.insert ( std::pair<std::string,int>(name,id) );
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
        ret.item.clear();           // in case of error, clean up partial data
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

db_reply < std::map <std::string, int> >
    get_dictionary_element_type
        (tntdb::Connection &conn)
{
    return get_dictionary(conn, st_dictionary_element_type);
}

db_reply < std::map <std::string, int> >
    get_dictionary_device_type
        (tntdb::Connection &conn)
{
    return get_dictionary(conn, st_dictionary_device_type);
}

// select basic information about asset element by name
db_reply <db_a_elmnt_t>
    select_asset_element_by_name
        (tntdb::Connection &conn,
         const char *element_name)
{
    LOG_START;
    log_debug ("  element_name = '%s'", element_name);

    db_a_elmnt_t item{0,"","",0,5,0,0};
    db_reply <db_a_elmnt_t> ret = db_reply_new(item);

    if ( !is_ok_name (element_name) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "name is not valid";
        log_error ("end: %s", ret.msg);
        return ret;
    }

    try {
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "   v.name, v.id_parent, v.status, v.priority, v.business_crit, v.id, v.id_type"
            " FROM"
            "   v_bios_asset_element v"
            " WHERE v.name = :name"
        );

        tntdb::Row row = st.set("name", element_name).
                            selectRow();
        
        row[0].get(ret.item.name);
        assert ( !ret.item.name.empty() );  // database is corrupted

        row[1].get(ret.item.parent_id);
        row[2].get(ret.item.status);
        row[3].get(ret.item.priority);
        row[4].get(ret.item.bc);
        row[5].get(ret.item.id);
        row[6].get(ret.item.type_id);
        
        ret.status = 1;
        LOG_END;
        return ret;
    } 
    catch (const tntdb::NotFound &e) {
        ret.status        = 0;
        ret.errtype       = DB_ERR;
        ret.errsubtype    = DB_ERROR_NOTFOUND;
        ret.msg           = "element with specified name was not found";
        LOG_END;
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

db_reply <std::vector<db_a_elmnt_t>>
    select_asset_elements_by_type
        (tntdb::Connection &conn,
         a_elmnt_tp_id_t type_id)
{
    LOG_START;

    std::vector<db_a_elmnt_t> item{};
    db_reply <std::vector<db_a_elmnt_t>> ret = db_reply_new(item);
    
    try{
        // Can return more than one row.
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "   v.name , v.id_parent, v.status, v.priority, v.business_crit, v.id"
            " FROM"
            "   v_bios_asset_element v"
            " WHERE v.id_type = :typeid"
        );
    
        tntdb::Result result = st.set("typeid", type_id).
                                  select();
        log_debug("[v_bios_asset_element]: were selected %" PRIu32 " rows",
                                                            result.size());

        // Go through the selected elements
        for ( auto &row: result )
        {
            db_a_elmnt_t m{0,"","",0,5,0,0};
            
            row[0].get(m.name);
            assert ( !m.name.empty() );  // database is corrupted

            row[1].get(m.parent_id);
            row[2].get(m.status);
            row[3].get(m.priority);
            row[4].get(m.bc);
            row[5].get(m.id);

            ret.item.push_back(m);
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

//=============================================================================
db_reply <std::set <std::pair<a_elmnt_id_t ,a_elmnt_id_t>>>
    select_links_by_container
        (tntdb::Connection &conn,
         a_elmnt_id_t element_id)
{
    LOG_START;
    log_debug ("  links are selected for element_id = %" PRIi32, element_id);
    a_lnk_tp_id_t linktype = INPUT_POWER_CHAIN;
    
    //      all powerlinks are included into "resultpowers"
    std::set <std::pair<a_elmnt_id_t ,a_elmnt_id_t>> item{};
    db_reply <std::set<std::pair<a_elmnt_id_t ,a_elmnt_id_t>>> ret = db_reply_new(item);

    try{
        // v_bios_asset_link are only devices, 
        // so there is no need to add more constrains
        tntdb::Statement st = conn.prepareCached(
            " SELECT"                
            "   v.id_asset_element_src,"
            "   v.id_asset_element_dest"              
            " FROM"
            "   v_bios_asset_link v,"
            "   v_bios_asset_element_super_parent v1,"
            "   v_bios_asset_element_super_parent v2"
            " WHERE"
            "   v.id_asset_link_type = :linktypeid AND"
            "   v.id_asset_element_dest = v2.id_asset_element AND"
            "   v.id_asset_element_src = v1.id_asset_element AND" 
            "   ("
            "       ( :containerid IN (v2.id_parent1, v2.id_parent2 ,v2.id_parent3,"
            "               v2.id_parent4) ) OR"
            "       ( :containerid IN (v1.id_parent1, v1.id_parent2 ,v1.id_parent3,"
            "               v1.id_parent4) )"
            "   )"
        );
        
        // can return more than one row
        tntdb::Result result = st.set("containerid", element_id).
                                  set("linktypeid", linktype).
                                  select();
        log_debug("[t_bios_asset_link]: were selected %" PRIu32 " rows",
                                                         result.size());

        // Go through the selected links
        for ( auto &row: result )
        {
            // id_asset_element_src, required
            a_elmnt_id_t id_asset_element_src = 0;
            row[0].get(id_asset_element_src);
            assert ( id_asset_element_src );
            
            // id_asset_element_dest, required
            a_elmnt_id_t id_asset_element_dest = 0;
            row[1].get(id_asset_element_dest);
            assert ( id_asset_element_dest );
            
            ret.item.insert(std::pair<a_elmnt_id_t ,a_elmnt_id_t>(id_asset_element_src, id_asset_element_dest));
        } // end for
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

// TODO redo into struct instead of tuple
db_reply <std::vector<device_info_t>>
    select_asset_device_by_container
        (tntdb::Connection &conn,
         a_elmnt_id_t element_id)
{
    LOG_START;
    log_debug ("container element_id = %" PRIi32, element_id);

    std::vector<device_info_t> item{};
    db_reply <std::vector<device_info_t>> ret = db_reply_new(item);
    
    try{
        // Can return more than one row.
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "   v.name,"
            "   v.id_asset_element, v.id_asset_device_type, v.type_name"
            " FROM"
            "   v_bios_asset_element_super_parent v"
            " WHERE :containerid in (v.id_parent1, v.id_parent2, v.id_parent3, v.id_parent4)"
        );
    
        tntdb::Result result = st.set("containerid", element_id).
                                  select();
        log_debug("[t_bios_asset_element]: were selected %" PRIu32 " rows",
                                                            result.size());

        // Go through the selected elements
        for ( auto &row: result )
        {
            std::string device_name = "";
            row[0].get(device_name);
            assert ( !device_name.empty() );
            
            a_elmnt_id_t device_asset_id = 0;
            row[1].get(device_asset_id);
            assert ( device_asset_id );

            a_dvc_tp_id_t device_type_id = 0;
            row[2].get(device_type_id);
            assert ( device_type_id );

            std::string device_type_name = "";
            row[3].get(device_type_name);
            assert ( !device_type_name.empty() );

            ret.item.push_back(std::make_tuple(device_asset_id, device_name, 
                                device_type_name, device_type_id));
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
