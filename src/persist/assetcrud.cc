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
#include "dbpath.h"
#include "assetcrud.h"
#include "monitor.h"
#include "persist_error.h"
#include "asset_types.h"

zmsg_t* asset_msg_process(zmsg_t **msg) {
    log_debug("Processing asset message in persistence layer\n");
    zmsg_t *result = NULL;

    asset_msg_t *amsg = asset_msg_decode(msg);
    if(amsg == NULL) {
        log_warning ("Malformed asset message received!");
        return common_msg_encode_fail(BAD_INPUT, BAD_INPUT_WRONG_INPUT,
	                                    "Malformed asset message message received!", NULL);src/persist/assertcrud.cc
    }

    int msg_id = asset_msg_id (amsg);
    
    switch (msg_id) {

        case ASSET_MSG_GET_ELEMENT: {
            // datacenter|room|row|rack|group|device
            result = get_asset_element(url.c_str(), amsg);
            break;
        }
        case ASSET_MSG_UPDATE_ELEMENT:
        case ASSET_MSG_INSERT_ELEMENT:
        case ASSET_MSG_DELETE_ELEMENT: {
            //not implemented yet
            break;
        }
        case ASSET_MSG_GET_ELEMENTS: {
            // datacenters|rooms|rows|racks|groups
            result = get_asset_elements(url.c_str(), amsg);
            break;
        }
        case ASSET_MSG_ELEMENT:
        case ASSET_MSG_RETURN_ELEMENT:
        case ASSET_MSG_OK:
        case ASSET_MSG_FAIL:
        case ASSET_MSG_RETURN_ELEMENTS:
        default: {src/persist/assertcrud.cc
            log_warning ("Wrong asset message (id %d) received!", msg_id);
            result = common_msg_encode_fail(BAD_INPUT, BAD_INPUT_WRONG_INPUT,
	                                        "Wrong asset message message received!", NULL);

            break;
        }
    }

    asset_msg_destroy(&amsg);
    return result;
};

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
        );src/persist/assertcrud.cc
        
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

zlist_t* select_asset_device_link(const char* ursrc/persist/assertcrud.ccl, 
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

m_dvc_id_t convert_asset_to_monitor(const char* url, 
                a_elmnt_id_t asset_element_id)
{
    assert ( asset_element_id );
    m_dvc_id_t       device_discovered_id = 0;
    a_elmnt_tp_id_t  element_type_id      = 0;
    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "   v.id_discovered_device, v1.id_type"
            " FROM"
            "   v_bios_monitor_asset_relation v"
            " LEFT JOIN t_bios_asset_element v1"
            "   ON (v1.id_asset_element = v.id_asset_element)"
            " WHERE v.id_asset_element = :id"
        );
        
        // TODO set 
        tntdb::Row row = st.setUnsigned32("id", asset_element_id).
                            selectRow();

        row[0].get(device_discovered_id);

        row[1].get(element_type_id);
        assert (element_type_id);
    }
    catch (const tntdb::NotFound &e){
        // apropriate asset element was not found
        log_info("asset element %d notfound %s ", asset_element_id,"end");
        throw bios::NotFound();
    }
    catch (const std::exception &e) {
        log_warning("abnormal %s ","end");
        throw bios::InternalDBError(e.what());
    }
    if ( element_type_id != asset_type::DEVICE )
    {
        log_info("specified element is not a device %s ","end");
        throw bios::ElementIsNotDevice();
    }
    else if ( device_discovered_id == 0 )
    {
        log_warning("monitor counterpart for the %d was not found %s ", 
                                                asset_element_id, "end");
        throw bios::MonitorCounterpartNotFound ();
    }
    log_info("end: asset element %d converted to %d ", asset_element_id, device_discovered_id);
    return device_discovered_id;
};

a_elmnt_id_t convert_monitor_to_asset(const char* url, 
                    m_dvc_id_t discovered_device_id)
{
    log_info("%s ","start");
    assert ( discovered_device_id );
    a_elmnt_id_t asset_element_id = 0;
    try{
        tntdb::Connection conn = tntdb::connectCached(url);
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            " id_asset_element"
            " FROM"
            " v_bios_monitor_asset_relation"
            " WHERE id_discovered_device = :id"
        );

        // TODO set 
        tntdb::Value val = st.setUnsigned32("id", discovered_device_id).
                              selectValue();

        val.get(asset_element_id);
    }
    catch (const tntdb::NotFound &e){
        // apropriate asset element was not found
        log_info("notfound %s ","end");
        throw bios::NotFound();
    }
    catch (const std::exception &e) {
        log_warning("abnormal %s ","end");
        throw bios::InternalDBError(e.what());
    }
    log_info("normal %s ","end");
    return asset_element_id;
};
