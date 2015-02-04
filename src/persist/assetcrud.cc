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

// Hash values must be printable strings; keys may not contain '='.
// The PROCESS of processing the messages on the database side is:
// --  Database returns a message as a reply. 
// --  But persistence logic should decide, if reply should be send back 
//          to client or not.

#include <exception>
#include <assert.h>

#include <czmq.h>
#include <tntdb/connect.h>
#include <tntdb/row.h>
#include <tntdb/result.h>
#include <tntdb/error.h>

#include "log.h"
#include "defs.h"
#include "dbpath.h"
#include "assetcrud.h"
#include "monitor.h"
#include "persist_error.h"
#include "asset_types.h"
#include "dbhelpers.h"

zlist_t* select_asset_element_groups(const char* url, 
       a_elmnt_id_t element_id)
{
    log_info("%s ","start");
    assert ( element_id );

    zlist_t* groups = zlist_new();
    zlist_autofree(groups);

    try {
        tntdb::Connection conn = tntdb::connectCached(url);

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
        tntdb::Result result = st_gr.setUnsigned32("idelement", element_id).
                                     select(); 
        // TODO move 10 to some constant
        char buff[10];

        // Go through the selected groups
        for ( auto &row: result )
        {
            // group_id, required
            a_elmnt_id_t group_id = 0;
            row[0].get(group_id);
            assert ( group_id != 0 );  // database is corrupted
            
            sprintf(buff, "%d", group_id);
            zlist_push (groups, buff);
        }
    }
    catch (const std::exception &e) {
        // internal error in database
        zlist_destroy (&groups);
        log_warning("abnormal %s ","end");
        return NULL;
    }
    log_info("normal %s ","end");
    return groups;
}

zlist_t* select_asset_device_link(const char* url, 
                a_elmnt_id_t device_id, a_lnk_tp_id_t link_type_id)
{
    log_info("%s ","start");
    
    assert ( device_id );
    assert ( link_type_id );

    zlist_t* links = zlist_new();
    assert ( links );

    zlist_autofree(links);
    
    try {
        tntdb::Connection conn = tntdb::connectCached(url);

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
        tntdb::Result result = st_pow.setUnsigned32("iddevice", device_id).
                                      set("idlinktype", link_type_id).
                                      select();
        // TODO move 26 to constants
        char buff[26];     // 10+3+3+10

        // Go through the selected links
        for ( auto &row: result )
        { 
            // element_id, required
            a_elmnt_id_t element_id = 0;
            row[0].get(element_id);
            assert ( element_id != 0 );  // database is corrupted

            // src_out
            a_lnk_src_out_t src_out;
            bool srcIsNotNull  = row[1].get(src_out);

            // dest_in
            a_lnk_dest_in_t dest_in;
            bool destIsNotNull = row[2].get(dest_in);

            sprintf(buff, "%d:%d:%d:%d", 
                srcIsNotNull ? src_out : SRCOUT_DESTIN_IS_NULL, 
                element_id, 
                destIsNotNull ? dest_in : SRCOUT_DESTIN_IS_NULL, 
                device_id);
            zlist_push(links, buff);
        }
    }
    catch (const std::exception &e) {
        // internal error in database
        zlist_destroy (&links);
        log_warning("abnormal %s ","end");
        return NULL;
    }
    log_info("normal %s ","end");
    return links;
}

zhash_t* select_asset_element_attributes(const char* url, 
                                         a_elmnt_id_t element_id)
{
    log_info("%s ","start");
    assert ( element_id );
    zhash_t* extAttributes = zhash_new();
    zhash_autofree(extAttributes);

    try {
        tntdb::Connection conn = tntdb::connectCached(url);
    
        // Can return more than one row
        tntdb::Statement st_extattr = conn.prepareCached(
            " SELECT"
            " v.keytag , v.value"
            " FROM"
            " v_bios_asset_ext_attributes v"
            " WHERE v.id_asset_element = :idelement"
        );

        // TODO set
        tntdb::Result result = 
                    st_extattr.setUnsigned32("idelement", element_id).
                               select();

        // Go through the selected extra attributes
        for (  auto &row: result )
        {
            // keytag, required
            std::string keytag = "";
            row[0].get(keytag);
            assert ( !keytag.empty() );  // database is corrupted

            // value , required
            std::string value = "";
            row[1].get(value);
            assert ( !value.empty() );   // database is corrupted


            // TODO type convertions
            zhash_insert (extAttributes, keytag.c_str(), 
                          (void*)value.c_str());
        }
    }
    catch (const std::exception &e) {
        // internal error in database
        zhash_destroy (&extAttributes);
        log_warning("abnormal %s ","end");
        return NULL;
    }
    log_info("normal %s ","end");
    return extAttributes;
}

zmsg_t* select_asset_device(const char* url, asset_msg_t** element, 
                            a_elmnt_id_t element_id)
{
    log_info ("%s ", "start");
    assert ( asset_msg_id (*element) == ASSET_MSG_ELEMENT );
    std::string mac = "";
    std::string ip = "";
    std::string hostname = "";
    std::string fqdn = "";
    a_dvc_tp_id_t id_asset_device_type = 0;
    std::string type_name = "";
    zlist_t* groups = NULL;
    zlist_t* powers = NULL;
    try {
        tntdb::Connection conn = tntdb::connectCached(url);
    
        // Get more attributes of the device
        // Can return one row or nothing 
        tntdb::Statement st_dev = conn.prepareCached(
            " SELECT"
            " v.mac , v.ip, v.hostname , v.full_hostname "
            "   , v.id_asset_device_type, v.name"
            " FROM"
            " v_bios_asset_device v"
            " WHERE v.id_asset_element = :idelement"
        );
    
        // TODO set 
        tntdb::Row row = st_dev.setUnsigned32("idelement", element_id).
                                selectRow();

        // mac
        row[0].get(mac);
        
        // ip
        row[1].get(ip);

        // hostname
        row[2].get(hostname);

        // fdqn
        row[3].get(fqdn);

        // id_asset_device_type, required
        row[4].get(id_asset_device_type);
        assert ( id_asset_device_type != 0 );  // database is corrupted
        
        // string representation of device type
        row[5].getString(type_name);
        assert ( !type_name.empty() );

        groups = select_asset_element_groups(url, element_id);

        if ( groups == NULL )    // internal error in database
        {
            log_warning("abnormal %s ","end");
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                "internal error during selecting groups occured", NULL);
        }
        powers = select_asset_device_link(url, element_id, INPUT_POWER_CHAIN);
        if ( powers == NULL )   // internal error in database
        {
            zlist_destroy (&groups);
            log_warning("abnormal %s ","end");
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                "internal error during selecting powerlinks occured", NULL);
        }
    }
    catch (const tntdb::NotFound &e) {
        // for every device in db there should be 
        // two rows
        // 1 in asset_element
        // 2 in asset_device
        asset_msg_destroy (element);
        zlist_destroy (&powers);
        zlist_destroy (&groups);

        log_warning("apropriate row in asset_device was not found %s",
                                                                "end");
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_BADINPUT, 
                                                        e.what(), NULL);
    }
    catch (const std::exception &e) {
        // internal error in database
        asset_msg_destroy (element);
        zlist_destroy (&powers);
        zlist_destroy (&groups);
        log_warning("abnormal %s ","end");
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
    }
    // device was found
    
    zmsg_t* nnmsg = asset_msg_encode (element);
    assert ( nnmsg );
    
    zmsg_t* msgdevice = asset_msg_encode_device (
                type_name.c_str(), groups, powers, ip.c_str(), 
                hostname.c_str(), fqdn.c_str(), mac.c_str(), nnmsg);
    assert ( msgdevice );

    zmsg_destroy (&nnmsg);
    zlist_destroy (&powers);
    zlist_destroy (&groups);

    log_info("normal %s ","end");
    return msgdevice;
}

zmsg_t* select_asset_element(const char* url, a_elmnt_id_t element_id, 
                              a_elmnt_tp_id_t element_type_id)
{
    log_info("%s ","start");
    assert ( element_id );
    assert ( element_type_id );
    a_elmnt_id_t parent_id = 0;
    a_elmnt_tp_id_t parent_type_id = 0;
    std::string name = "";

    try {
        tntdb::Connection conn = tntdb::connectCached(url);

        // Can return one row or nothing.
        // Get basic attributes of the element
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            " v.name , v.id_parent, v.id_parent_type"
            " FROM"
            " v_bios_asset_element v"
            " WHERE v.id = :id AND v.id_type = :typeid"
        );
        // TODO set 
        tntdb::Row row = st.setUnsigned32("id", element_id).
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
           
    zhash_t* extAttributes = select_asset_element_attributes(url, element_id);
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

zmsg_t* get_asset_element(const char *url, asset_msg_t *msg)
{
    log_info("%s ","start");
    assert ( msg );
    assert ( asset_msg_id (msg) == ASSET_MSG_GET_ELEMENT );

    const a_elmnt_id_t    element_id      = asset_msg_element_id (msg); 
    const a_elmnt_tp_id_t element_type_id = asset_msg_type (msg);
      
    zmsg_t* msgelement = 
                select_asset_element (url, element_id, element_type_id);
    
    if ( is_common_msg(msgelement) )  
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
        msgelement = select_asset_device(url, &returnelement, element_id);
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
          
    // make ASSET_MSG_RETURN_ELEMENT
    zmsg_t* resultmsg = asset_msg_encode_return_element 
                    (element_id, msgelement);
    assert ( resultmsg );
    zmsg_destroy (&msgelement);
    log_info("normal %s ","end");
    return resultmsg;
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

        // TODO move to constant
        char buff[16];

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
    
            // TODO type convertions
            sprintf(buff, "%d", id);
            zhash_insert(elements, buff, (void*)name.c_str());
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

/*
// because we want to be prepared to transaction processing, so we need 
// the same connection for all activities
common_msg_t* insert_asset_element (tntdb::Connection &conn, 
                                    const char *element_name, 
                                    a_elmnt_tp_id_t element_type_id,
                                    a_elmnt_id_t    parent_id)
{
    log_info ("start");
    // input parameters control 
    if ( !is_ok_name_length (element_name) )
    {
        log_error ("end: ignore insert, too long name");
        return generate_db_fail (DB_ERROR_BADINPUT, "too long name", NULL);
    }
    if ( !is_ok_element_type (element_type_id) )
    {
        log_error ("end: ignore insert, %d wrong element type", 
                                                        element_type_id);
        return generate_db_fail (DB_ERROR_BADINPUT,
                                            "unknown element type", NULL);
    }
    // ASSUMPTION: all datacenters are unlockated elements
    if ( ( element_type_id == asset_type::DATACENTER ) && 
         ( parent_id != 0 ) )
    {
        log_error ("end: ignore insert, datacentes are unlockated elements");
        return generate_db_fail (DB_ERROR_BADINPUT, "dataceter is not "
                                            "an unlockated element", NULL);
    }
    log_debug ("input parameters are correct");

    a_elmnt_id_t newid = 0;
    a_elmnt_id_t n     = 0; // number of rows affected

    try{
        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            "   v_bios_asset_element"
            "   (id, name, id_type, id_parent)"
            " VALUES (NULL, :name, :type, :parent)"
        );
   
        if ( parent_id == 0 )
            n = st.set("name", element_name).
                   set("type", element_type_id).
                   setNull("parent").
                   execute();
        else
            n = st.set("name", element_name).
                   set("type", element_type_id).
                   set("parent", parent_id).
                   execute();
        newid = conn.lastInsertId();
        log_debug("was inserted %d rows", n);
        // attention: 
        //  -- 0 rows can be inserted
        //        - because parent_id is not valid
        //        - there is no free space
        //        - in some other, but not normal cases
        //  -- 1 row is inserted - a usual case
        //  -- more than one row, it is not normal and it is not expected 
        //       due to nature of the insert statement 
    }
    catch (const std::exception &e) {
        log_warning ("end: abnormal");
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
    {
        log_info ("end: normal");
        return generate_ok (newid);
    }
    else
    {
        log_info ("end: %d - unexpected number of rows returned", n);
        return generate_db_fail (DB_ERROR_BADINPUT, 
                            "unexpected number of returned rows", NULL);
    }
}

common_msg_t*  insert_ext_attribute_one (tntdb::Connection &conn,
                                         const char   *value,
                                         const char   *keytag,
                                         a_elmnt_id_t  asset_element_id)
{
    log_info ("start");
    // input parameters control 
    if ( asset_element_id == 0 )
    {
        log_error ("end: ignore insert, apropriate asset element is "
                                                         "not specified");
        return generate_db_fail (DB_ERROR_BADINPUT, 
                        "apropriate asset element is not specified", NULL);
    }
    if ( !is_ok_value (value) )
    {
        log_error ("end: ignore insert, unexeptable value '%s'", value);
        return generate_db_fail (DB_ERROR_BADINPUT, 
                        "unexepetable value", NULL);
    }

    if ( !is_ok_keytag (keytag) )
    {
        log_error ("end: ignore insert, unexeptable keytag '%s'", keytag);
        return generate_db_fail (DB_ERROR_BADINPUT,
                        "unknown element type", NULL);
    }
    log_debug ("input parameters are correct");

    a_ext_attr_id_t newid = 0;
    a_ext_attr_id_t n     = 0; // number of rows affected

    try{
        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            "   v_bios_asset_ext_attributes"
            "   (id, keytag, value, id_asset_element)"
            " VALUES (NULL, :keytag, :value, :element)"
        );
   
        n = st.set("keytag" , keytag).
               set("value"  , value).
               set("element", asset_element_id).
               execute();
        newid = conn.lastInsertId();
        log_debug("was inserted %d rows", n);
        // attention: 
        //  -- 0 rows can be inserted
        //        - there is no free space
        //        - FK on id_asset_element
        //        - in some other, but not normal cases
        //  -- 1 row is inserted - a usual case
        //  -- more than one row, it is not normal and it is not expected 
        //       due to nature of the insert statement 
    }
    catch (const std::exception &e) {
        log_warning ("end: abnormal");
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
    {
        log_info ("end: normal");
        return generate_ok (newid);
    }
    else
    {
        log_info ("end: %d - unexpected number of rows returned", n);
        return generate_db_fail (DB_ERROR_BADINPUT, 
                        "unexpected number of returned rows", NULL);
    }
}

common_msg_t*  insert_into_group (tntdb::Connection &conn,
                                  a_elmnt_id_t  group_id,
                                  a_elmnt_id_t  asset_element_id)
{
    log_info ("start");
    // input parameters control 
    if ( asset_element_id == 0 )
    {
        log_error ("end: ignore insert, apropriate asset element is "
                                                            "not specified");
        return generate_db_fail (DB_ERROR_BADINPUT, 
                        "apropriate asset element is not specified", NULL);
    }
    if ( group_id == 0 )
    {
        log_error ("end: ignore insert, group is not specified");
        return generate_db_fail (DB_ERROR_BADINPUT, 
                        "group is not specified", NULL);
    }
    log_debug ("input parameters are correct");

    a_grp_rltn_id_t newid = 0;
    a_grp_rltn_id_t n     = 0; // number of rows affected

    try{
        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            "   v_bios_asset_group_relation"
            "   (id, id_asset_group, id_asset_element)"
            " VALUES (NULL, :group, :element)"
        );
   
        n = st.set("group"  , group_id).
               set("element", asset_element_id).
               execute();
        newid = conn.lastInsertId();
        log_debug("was inserted %d rows", n);
        // attention: 
        //  -- 0 rows can be inserted
        //        - there is no free space
        //        - FK on id_asset_element
        //        - FK on id_asset_group
        //        - in some other, but not normal cases
        //  -- 1 row is inserted - a usual case
        //  -- more than one row, it is not normal and it is not expected
        //       due to nature of the insert statement 
    }
    catch (const std::exception &e) {
        log_warning ("end: abnormal");
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
    {
        log_info ("end: normal");
        return generate_ok (newid);
    }
    else
    {
        log_info ("end: %d - unexpected number of rows returned", n);
        return generate_db_fail (DB_ERROR_BADINPUT, 
                            "unexpected number of returned rows", NULL);
    }
}

common_msg_t*  insert_into_asset_device (tntdb::Connection &conn,
                                  a_elmnt_id_t   asset_element_id,
                                  const char    *hostname,
                                  const char    *fullhostname,
                                  const char    *ip,
                                  const char    *mac,
                                  a_dvc_tp_id_t  asset_device_type_id)
{
    log_info ("start");
    // input parameters control 
    if ( asset_element_id == 0 )
    {
        log_error ("end: ignore insert, apropriate asset element is "
                                                        "not specified");
        return generate_db_fail (DB_ERROR_BADINPUT, 
                        "apropriate asset element is not specified", NULL);
    }
    if ( !is_ok_asset_device_type (asset_device_type_id) )
    {
        log_error ("end: ignore insert, device type is not specified");
        return generate_db_fail (DB_ERROR_BADINPUT, 
                        "device type is not specified", NULL);
    }
    if ( !is_ok_hostname (hostname) )
    {
        log_error ("end: ignore insert, hostname is too long");
        return generate_db_fail (DB_ERROR_BADINPUT, 
                        "too long hostname", NULL);
    }
    if ( !is_ok_fullhostname (fullhostname) )
    {
        log_error ("end: ignore insert, fullhost name is too long");
        return generate_db_fail (DB_ERROR_BADINPUT, 
                        "too log full host name", NULL);
    }
    if ( !is_ok_ip (ip) )
    {
        log_error ("end: ignore insert, bad ip address");
        return generate_db_fail (DB_ERROR_BADINPUT, "bad ip address", NULL);
    }
    if ( !is_ok_mac (mac) )
    {
        log_error ("end: bad mac address");
        return generate_db_fail (DB_ERROR_BADINPUT, "bad mac address", NULL);
    }
    log_debug ("input parameters are correct");

    a_dvc_id_t newid = 0;
    a_dvc_id_t n     = 0; // number of rows affected

    try{
        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            "   v_bios_asset_device"
            "   (id, id_asset_element, id_asset_device_type,"
            "        hostname, fullhostname, ip, mac)"
            " VALUES"
            "   (NULL, :element, :type, :hostname,"
            "       :fullhostname, :ip, :mac)"
        );
   
        n = st.set("element", asset_element_id).
               set("type", asset_device_type_id).
               set("hostname", (hostname == NULL ? "": hostname) ).
               set("fullhostname", (fullhostname == NULL ? "": fullhostname)).
               set("ip", (ip == NULL ? "": ip) ).
               set("mac", (mac == NULL ? "": mac) ).
               execute();
        newid = conn.lastInsertId();
        log_debug("was inserted %d rows", n);
        // attention: 
        //  -- 0 rows can be inserted
        //        - FK on id_asset_element
        //        - there is no free space
        //        - in some other, but not normal cases
        //  -- 1 row is inserted - a usual case
        //  -- more than one row, it is not normal and it is not expected
        //       due to nature of the insert statement 
    }
    catch (const std::exception &e) {
        log_warning ("end: abnormal");
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
    {
        log_info ("end: normal");
        return generate_ok (newid);
    }
    else
    {
        log_info ("end: %d - unexpected number of rows returned", n);
        return generate_db_fail (DB_ERROR_BADINPUT, 
                            "unexpected number of returned rows", NULL);
    }
}

common_msg_t*  insert_into_asset_link (tntdb::Connection &conn,
                                       a_dvc_id_t      dvc_element_src_id,
                                       a_dvc_id_t      dvc_element_dest_id,
                                       a_lnk_tp_id_t   link_type_id,
                                       a_lnk_src_out_t src_out,
                                       a_lnk_dest_in_t dest_in)
{
    log_info ("start");
    // input parameters control 
    if ( dvc_element_dest_id == 0 )
    {
        log_error ("end: ignore insert, apropriate dest device is "
                                                    "not specified");
        return generate_db_fail (DB_ERROR_BADINPUT,
                        "apropriate dest device is not specified", NULL);
    }
    if ( dvc_element_src_id == 0 )
    {
        log_error ("end: ignore insert, apropriate src device is "
                                                    "not specified");
        return generate_db_fail (DB_ERROR_BADINPUT,
                        "apropriate src device is not specified", NULL);
    }
    if ( !is_ok_link_type (link_type_id) )
    {
        log_error ("end: ignore insert, bad link type");
        return generate_db_fail (DB_ERROR_BADINPUT, "bad link type", NULL);
    }
    // src_out and dest_in can take any value from available range
    log_debug ("input parameters are correct");

    a_lnk_id_t newid = 0;
    a_lnk_id_t n     = 0; // number of rows affected

    try{
        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            "   v_bios_asset_link"
            "   (id, id_asset_device_src, id_asset_device_dest,"
            "        id_asset_link_type, src_out, dest_in)"
            " VALUES (NULL, :dvcsrc, :dvcdest, :linktype, :out, :in)"
        );
        
        if ( src_out == 0 )
            st.setNull("out");
        else
            st.set("out", src_out);
        if ( dest_in == 0 )
            st.setNull("in");
        else
            st.set("in", dest_in);

        n = st.set("dvcsrc", dvc_element_src_id).
               set("dvcdest", dvc_element_dest_id).
               set("linktype", link_type_id).
               execute();
        newid = conn.lastInsertId();
        log_debug("was inserted %d rows", n);
        // attention: 
        //  -- 0 rows can be inserted
        //        - FK on id_asset_element
        //        - there is no free space
        //        - in some other, but not normal cases
        //  -- 1 row is inserted - a usual case
        //  -- more than one row, it is not normal and it is not expected
        //       due to nature of the insert statement 
    }
    catch (const std::exception &e) {
        log_warning ("end: abnormal");
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
    {
        log_info ("end: normal");
        return generate_ok (newid);
    }
    else
    {
        log_info ("end: %d - unexpected number of rows returned", n);
        return generate_db_fail (DB_ERROR_BADINPUT, 
                            "unexpected number of returned rows", NULL);
    }
}


// hash is untouched
common_msg_t* insert_ext_attributes (tntdb::Connection &conn, 
                                     zhash_t      *attributes,
                                     a_elmnt_id_t  asset_element_id)
{
    log_info ("start");
    // input parameters control 
    if ( asset_element_id == 0 )
    {
        log_error ("end: ignore insert, apropriate asset element is "
                                                            "not specified");
        return generate_db_fail (DB_ERROR_BADINPUT,
                        "apropriate asset element is not specified", NULL);
    }
    if ( attributes == NULL )
    {
        log_error ("end: ignore insert, ext attributes are "
                                                    "not specified (NULL)");
        return generate_db_fail (DB_ERROR_BADINPUT,
                        "ext attributes are not specified (NULL)", NULL);
    }
    if ( zhash_size (attributes) == 0 )
    {
        log_info ("end: nothing to insert");
        // actually, if there is nothing to insert, then insert was ok :)
        // but we need to return an id, so the only available non valid 
        // value is zero.
        return generate_ok (0);
    }
    log_debug ("input parameters are correct");

    char *value = (char *) zhash_first  (attributes);   // first value
    char *key   = (char *) zhash_cursor (attributes);   // key of this value
    
    // there is no supported bulk operations, 
    // so if there is more than one ext 
    // atrtribute we will insert them all iteratevely
    // the hash "attributes" is a finite set, so the cycle will 
    // end in finite number of steps
    common_msg_t* reply = NULL;
    
    while ( true )
    {
        reply = insert_ext_attribute_one (conn, value, key, asset_element_id);

        value = (char *) zhash_next   (attributes);   // next value
        key   = (char *) zhash_cursor (attributes);   // key of this value
       
        // There is no need to continue
        //      - if insert was unsuccsessful
        //      - if we reach the end of the hash
        if ( common_msg_id (reply) != COMMON_MSG_DB_OK )
        {
            // reply is not destroyed, because we want to buble it 
            // to upper level
            log_error ("end: insert of ext attributes was interrupted "
                                                            "in the middle");
            break;
        }
        else if ( value == NULL )
        {
            // reply is not destroyed, because we want to buble it to 
            // upper level
            log_info ("end: all ext attributes were inserted successfully");
            break;
        }
        else
            // in this case we are going to insert more rows, so 
            // ignore successful midresults
            common_msg_destroy (&reply);
    }
    return reply;
}

common_msg_t* insert_basic_info(tntdb::Connection &conn, 
                                const char *element_name, 
                                a_elmnt_tp_id_t element_type_id,
                                a_elmnt_id_t    parent_id,
                                zhash_t        *attributes)
{
    log_info ("start");
    common_msg_t* response = insert_asset_element(conn, element_name,
                                        element_type_id, parent_id);
            
    auto response_msg_id = common_msg_id (response);
    if ( response_msg_id != COMMON_MSG_DB_OK )
    {
        common_msg_t  *response1 = insert_ext_attributes 
                        (conn, attributes, common_msg_rowid (response));
        
        auto response_msg_id1 = common_msg_id (response1);
        if ( response_msg_id1 != COMMON_MSG_DB_OK )
        {   
            // insert was partial: t_bios_asset_element - ok;
            //                     t_bios_asset_ext_attributes - not ok;
            // TODO: What we should return?
            // how it works now: ignore the response1, create new response, 
            // that indicates a partial insert
            //response = common_msg_new (COMMON_MSG_FAIL);
            //common_msg_set_errtype (response, ??? );
            //common_msg_set_errorno (response, ??? );
            //common_msg_set_errmsg  (response, "");
            //common_msg_set_erraux  (response, NULL);    
            //// TODO: should we return old response here???
            common_msg_destroy (&response1);
            log_warning ("end: Partial insert (expected)");
            return PARTIAL_INSERT;
        }
    }
    else if ( response_msg_id != COMMON_MSG_FAIL )
    {
        log_error ("unexpected type of common_msg had been returned");
        // TODO: What should we do in this situation?
        // how it works now: ignore previous response, 
        // create new one of type common_msg_fail
        common_msg_destroy (&response);
        response = common_msg_new (COMMON_MSG_FAIL);
        common_msg_set_errtype (response, INTERNAL_ERR);
        common_msg_set_errorno (response, INTERNAL_UNEXPECTED_RETURN_TYPE);
        common_msg_set_errmsg  (response, "unexpected type of common_msg"
                                                    " had been returned");
        common_msg_set_erraux  (response, NULL);    
        // TODO: should we return old response here???
    }
    // want to return id of inserted element, but not the id of 
    // some ext_attribute
    return response; 
}
*/
