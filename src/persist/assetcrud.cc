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


zlist_t* select_asset_element_groups(tntdb::Connection &conn, 
       a_elmnt_id_t element_id)
{
    log_info("%s ","start");
    assert ( element_id );

    zlist_t* groups = zlist_new();
    zlist_autofree(groups);

    try {
        // Get information about the groups element belongs to
        // Can return more than one row
        tntdb::Statement st_gr = conn.prepareCached(
            " SELECT"
            " v.id_asset_group"
            " FROM"
            " v_bios_asset_group_relation v"
            " WHERE v.id_asset_element = :idelement"
        );
        
        // TODO set 
        tntdb::Result result = st_gr.set("idelement", element_id).
                                     select(); 
        // Go through the selected groups
        for ( auto &row: result )
        {
            // group_id, required
            a_elmnt_id_t group_id = 0;
            row[0].get(group_id);
            assert ( group_id != 0 );  // database is corrupted
            
            zlist_push (groups, (char *)std::to_string(group_id).c_str());
        }
    }
    catch (const std::exception &e) {
        // internal error in database
        zlist_destroy (&groups);
        log_warning("abnormal end\n e.what(): %s ", e.what());
        return NULL;
    }
    log_info("normal %s ","end");
    return groups;
}

zlist_t* select_asset_device_links_to(tntdb::Connection &conn, 
                a_elmnt_id_t device_id, a_lnk_tp_id_t link_type_id)
{
    log_info("%s ","start");
    
    assert ( device_id );
    assert ( link_type_id );

    zlist_t* links = zlist_new();
    assert ( links );

    zlist_autofree(links);
    
    try {
        // Get information about the links the specified device 
        // belongs to
        // Can return more than one row
        tntdb::Statement st_pow = conn.prepareCached(
            " SELECT"
            " v.id_asset_element_src , v.src_out , v.dest_in"
            " FROM"
            " v_bios_asset_link v"
            " WHERE v.id_asset_element_dest = :iddevice "
            "       AND v.id_asset_link_type = :idlinktype"
        ); 
        
        // TODO set
        tntdb::Result result = st_pow.set("iddevice", device_id).
                                      set("idlinktype", link_type_id).
                                      select();

        // Go through the selected links
        for ( auto &row: result )
        { 
            // element_id, required
            a_elmnt_id_t element_id = 0;
            row[0].get(element_id);
            assert ( element_id != 0 );  // database is corrupted

            // src_out
            std::string src_out = SRCOUT_DESTIN_IS_NULL;
            row[1].get(src_out);

            // dest_in
            std::string dest_in = SRCOUT_DESTIN_IS_NULL;
            row[2].get(dest_in);

            zlist_push(links, (char *)(src_out + ":"
                                 + std::to_string (element_id) + ":"
                                 + dest_in + ":"
                                 + std::to_string (device_id) ).c_str());
        }
    }
    catch (const std::exception &e) {
        // internal error in database
        zlist_destroy (&links);
        log_warning("abnormal end\n e.what(): %s ", e.what());
        return NULL;
    }
    log_info("normal %s ","end");
    return links;
}


// workaround:
// need to put (key, value, read_only) INTO HASH
// so, key to key
// value = w:value when read_only is 0
// value = r:value when read_only is 1
zhash_t* select_asset_element_attributes(tntdb::Connection &conn, 
                                         a_elmnt_id_t element_id)
{
    LOG_START;
    assert ( element_id );
    zhash_t* extAttributes = zhash_new();
    zhash_autofree(extAttributes);

    try {
        // Can return more than one row
        tntdb::Statement st_extattr = conn.prepareCached(
            " SELECT"
            "   v.keytag, v.value, v.read_only"
            " FROM"
            "   v_bios_asset_ext_attributes v"
            " WHERE v.id_asset_element = :idelement"
        );

        tntdb::Result result = 
                    st_extattr.set("idelement", element_id).
                               select();

        // Go through the selected extra attributes
        for ( auto &row: result )
        {
            // keytag, required
            std::string keytag = "";
            row[0].get(keytag);
            assert ( !keytag.empty() );  // database is corrupted

            // value , required
            std::string value = "";
            row[1].get(value);
            assert ( !value.empty() );   // database is corrupted
         
            // value , required
            int read_only = 0;
            row[2].get(read_only);

            zhash_insert (extAttributes, keytag.c_str(), 
                          (void*) (( read_only ? "r:" : "w:")+value).c_str());
        }
    }
    catch (const std::exception &e) {
        // internal error in database
        zhash_destroy (&extAttributes);
        log_warning("abnormal end\n e.what(): %s ", e.what());
        return NULL;
    }
    log_info("normal %s ","end");
    return extAttributes;
}


zmsg_t* select_asset_device (tntdb::Connection &conn, a_elmnt_id_t element_id)
{
    log_info ("start");
    log_debug ("asset_element_id = %" PRIu32, element_id);
    assert ( element_id );
    
//    std::string mac = "";
//    std::string ip = "";
//    std::string hostname = "";
//    std::string fqdn = "";
    std::string type_name = "";
    a_dvc_tp_id_t id_asset_device_type = 0;

    try {
        // Can return one row or nothing 
        tntdb::Statement st_dev = conn.prepareCached(
            " SELECT"
//            "   v.mac, v.ip, v.hostname, v.full_hostname,"
            "   v.id_asset_device_type, v.name"
            " FROM"
            "   v_bios_asset_device v"
            " WHERE"
            "   v.id_asset_element = :idelement"
        );
    
        tntdb::Row row = st_dev.set("idelement", element_id).
                                selectRow();

        // mac
//        row[0].get(mac);
        
        // ip
//        row[1].get(ip);

        // hostname
//        row[2].get(hostname);

        // fdqn
//        row[3].get(fqdn);

        // id_asset_device_type, required
        row[0].get(id_asset_device_type);
        assert ( id_asset_device_type != 0 );  // database is corrupted
        
        // string representation of device type
        row[1].getString(type_name);
        assert ( !type_name.empty() );

        return  asset_msg_encode_device (
                type_name.c_str(), NULL, NULL, NULL, 
                NULL, NULL, NULL, NULL);
    }
    catch (const tntdb::NotFound &e) {
        log_warning("end: apropriate row in asset_device was not found");
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_NOTFOUND, 
                                                        e.what(), NULL);
    }
    catch (const std::exception &e) {
        // internal error in database
        log_warning("end: abnormal with '%s'", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
    }
}



zmsg_t* extend_asset_element2device(tntdb::Connection &conn, asset_msg_t** element, 
                            a_elmnt_id_t element_id)
{
    log_info ("%s ", "start");
    assert ( asset_msg_id (*element) == ASSET_MSG_ELEMENT );
    std::string mac = "";
    std::string ip = "";
    std::string hostname = "";
    std::string fqdn = "";
    std::string type_name = "";

    zmsg_t* adevice = select_asset_device (conn, element_id);
    if ( is_common_msg(adevice) )
    {
        asset_msg_destroy (element);
        return adevice;
    }
    else
    {
        // device was found
        zlist_t *groups = select_asset_element_groups(conn, element_id);
        if ( groups == NULL )    // internal error in database
        {
            zmsg_destroy (&adevice);
            asset_msg_destroy (element);
            log_warning("end: abnormal groups == NULL , element_id: %" PRIu32, element_id);
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                "internal error during selecting groups occured", NULL);
        }
        zlist_t *powers = select_asset_device_links_to(conn, element_id, INPUT_POWER_CHAIN);
        if ( powers == NULL )   // internal error in database
        {
            zlist_destroy (&groups);
            asset_msg_destroy (element);
            zmsg_destroy  (&adevice);
            log_warning("end: abnormal powers == NULL element_id: %" PRIu32, element_id);
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                "internal error during selecting powerlinks occured", NULL);
        }

        zmsg_t *nnmsg = asset_msg_encode (element);
        assert ( nnmsg );
    
        asset_msg_t *adevice_decode = asset_msg_decode (&adevice);
        asset_msg_set_powers (adevice_decode, &powers);
        asset_msg_set_groups (adevice_decode, &groups);
        asset_msg_set_msg (adevice_decode, &nnmsg);

        zmsg_destroy (&nnmsg);
        zlist_destroy (&powers);
        zlist_destroy (&groups);

        log_info("end: normal");
        return asset_msg_encode (&adevice_decode);
    }
}

zmsg_t* select_asset_element(tntdb::Connection &conn, a_elmnt_id_t element_id, 
                              a_elmnt_tp_id_t element_type_id)
{
    log_info("%s ","start");
    assert ( element_id );
    assert ( element_type_id );
    a_elmnt_id_t parent_id = 0;
    a_elmnt_tp_id_t parent_type_id = 0;
    std::string name = "";

    try {
        // Can return one row or nothing.
        // Get basic attributes of the element
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            " v.name , v.id_parent, v.id_parent_type"
            " FROM"
            " v_bios_asset_element v"
            " WHERE v.id = :id AND v.id_type = :typeid"
        );
        tntdb::Row row = st.set("id", element_id).
                            set("typeid", element_type_id).
                            selectRow();
        
        // element was found
        // name, is required
        row[0].get(name);
        assert ( !name.empty() );  // database is corrupted

        // parent_id
        row[1].get(parent_id);

        // parent_type_id, required, if parent_id != 0
        row[2].get(parent_type_id);
        assert ( ! ( ( parent_type_id == 0 ) && (parent_id != 0) ) ); 
        // database is corrupted
    } 
    catch (const tntdb::NotFound &e) {
        // element with specified type was not found
        log_info("notfound %s ","end");
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_NOTFOUND, 
                                                    e.what(), NULL);
    }
    catch (const std::exception &e) {
        // internal error in database 
        log_warning("abnormal %s ","end");
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                    e.what(), NULL);
    }
           
    zhash_t* extAttributes = select_asset_element_attributes(conn, element_id);
    if ( extAttributes == NULL )    // internal error in database
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL,
          "internal error during selecting ext attributes occured", NULL);

    zmsg_t* msgelement = asset_msg_encode_element
            (name.c_str(), parent_id, parent_type_id, 
             element_type_id, extAttributes);
    assert ( msgelement );

    zhash_destroy (&extAttributes);
    log_info("normal %s ","end");
    return msgelement;
}

// GET functions

db_reply_t
    process_insert_inventory
        (tntdb::Connection &conn, const char *device_name, zhash_t *ext_attributes)
{
    LOG_START;
    db_reply_t ret = db_reply_new();
        
    tntdb::Transaction trans (conn);

    ret = select_device (conn, device_name);
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


zmsg_t* get_asset_element(const char *url, asset_msg_t *msg)
{
    log_info("%s ","start");
    assert ( msg );
    assert ( asset_msg_id (msg) == ASSET_MSG_GET_ELEMENT );

    const a_elmnt_id_t    element_id      = asset_msg_element_id (msg); 
    const a_elmnt_tp_id_t element_type_id = asset_msg_type (msg);
    try{
        
        tntdb::Connection conn = tntdb::connectCached(url);
        
        zmsg_t* msgelement = 
            select_asset_element (conn, element_id, element_type_id);

        if ( is_common_msg (msgelement) )
        {
            // element was not found  or error occurs
            log_info("errors occured in subroutine %s ","end");
            return msgelement;
        }
        // element was found
        if ( element_type_id == asset_type::DEVICE )
        {
            log_debug ("%s ", "start looking for device");
            // destroys msgelement
            asset_msg_t* returnelement = asset_msg_decode (&msgelement);
            msgelement = extend_asset_element2device(conn, &returnelement, element_id);
            assert ( msgelement );
            assert ( returnelement == NULL );

            if ( is_common_msg (msgelement) )
            {
                // because this element has asset_type::DEVICE type, then 
                // this should never happen

                // TODO should we inform user through the error_id about it??
                log_error ("%s ", "inconsistent db state, end");
                return msgelement;
            }

            log_debug ("%s ", "end looking for device");
            // device was found
        }
        // TODO rework this function
        // make ASSET_MSG_RETURN_ELEMENT
        zmsg_t* resultmsg = asset_msg_encode_return_element 
                    (element_id, msgelement);
        assert ( resultmsg );
        zmsg_destroy (&msgelement);
        log_info("normal %s ","end");
        return resultmsg;
    }
    catch (const std::exception &e) {
        log_warning ("end: abnormal with '%s'", e.what());
        return common_msg_encode_fail (DB_ERR, DB_ERROR_INTERNAL, e.what(), NULL);
    }
}

zmsg_t* get_asset_elements(const char *url, asset_msg_t *msg)
{
    log_info("%s ","start");
    assert ( msg );
    assert ( asset_msg_id (msg) == ASSET_MSG_GET_ELEMENTS );

    zhash_t *elements = zhash_new();
    zhash_autofree(elements);

    try{
        const a_elmnt_tp_id_t element_type_id = asset_msg_type (msg);
     
        tntdb::Connection conn = tntdb::connectCached(url);

        // Can return more than one row.
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            " v.name, v.id"
            " FROM"
            " v_bios_asset_element v"
            " WHERE v.id_type = :typeid"
        );
    
        // TODO set 
        tntdb::Result result = st.set("typeid", element_type_id).
                                  select();

        if ( result.size() == 0 )  // elements were not found
        {
            log_info("notfound %s ","end");
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_NOTFOUND, 
                       "elements of speified type were not found", NULL);
        }

        // Go through the selected elements
        for ( auto &row: result )
        {
            // name, is required
            std::string name = "";
            row[0].get(name);
            assert ( !name.empty() );  // database is corrupted

            // id, is required
            a_elmnt_id_t id = 0;
            row[1].get(id);
            assert( id != 0);    // database is corrupted
    
            zhash_insert(elements,(char *)std::to_string(id).c_str(), (void*)name.c_str());
        }
    }
    catch (const std::exception &e)
    {
        // internal error in database
        zhash_destroy (&elements);
        log_warning("abnormal %s ","end");
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                    e.what(), NULL);
    }
  
    // make ASSET_MSG_RETURN_ELEMENTS
    zmsg_t *resultmsg = asset_msg_encode_return_elements (elements);
    assert(resultmsg);

    zhash_destroy (&elements);
    log_info("normal %s ","end");
    return resultmsg;
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
            "   v.name , v.id_parent, v.status, v.priority, v.business_crit, v.id"
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

