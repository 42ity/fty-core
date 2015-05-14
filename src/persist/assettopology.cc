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

/*! \file assettopology.cc
    \brief Functions for getting the topology (location and power) from the
           database
    \author Alena Chernikava <alenachernikava@eaton.com>
*/

#include <cassert>
#include <cstring>
#include <set>
#include <tuple>

#include <czmq.h>
#include <tntdb/connect.h>
#include <tntdb/row.h>
#include <tntdb/error.h>
#include <tntdb/value.h>
#include <tntdb/result.h>

#include "log.h"
#include "assetcrud.h"
#include "common_msg.h"
#include "asset_types.h"

#include "assettopology.h"
#include "persist_error.h"
#include "cleanup.h"

// TODO HARDCODED CONSTANTS for asset device types

// TODO This parameter should be placed in configure file
// but now configure file doesn't exist. 
// So instead of it the constat would be used
#define MAX_RECURSION_DEPTH 6
#define INPUT_POWER_CHAIN 1

zmsg_t *process_assettopology (const char *database_url,
                        asset_msg_t **message_p) {
    log_open ();
    log_set_level (LOG_DEBUG);

    assert (message_p);
    assert (database_url);
    zmsg_t *return_msg = NULL;
    if (*message_p) {
        asset_msg_t *message = *message_p;
        assert (message);
//        asset_msg_print(message);
        int id = asset_msg_id (message);
        switch (id) {
            // TODO: Actually some of these messages don't make sense to 
            //       be implemented (like ASSET_MSG_RETURN_LOCATION_TO),
            //       someone should sort them out and make a common zmsg 
            //       errmsg for them. 
            case ASSET_MSG_ELEMENT:
            case ASSET_MSG_DEVICE:
            case ASSET_MSG_GET_ELEMENT:
            case ASSET_MSG_RETURN_ELEMENT:
            case ASSET_MSG_UPDATE_ELEMENT:
            case ASSET_MSG_INSERT_ELEMENT:
            case ASSET_MSG_DELETE_ELEMENT:
            case ASSET_MSG_OK:
            case ASSET_MSG_FAIL:
            case ASSET_MSG_GET_ELEMENTS:
            case ASSET_MSG_RETURN_ELEMENTS:
            case ASSET_MSG_POWERCHAIN_DEVICE:
            case ASSET_MSG_RETURN_POWER:
            case ASSET_MSG_RETURN_LOCATION_TO:
            case ASSET_MSG_RETURN_LOCATION_FROM:
            {
                log_info ("Processing of messages with ID = '%d' " 
                    "not implemented at the moment.\n", id);
                _scoped_common_msg_t *common_msg = common_msg_new (COMMON_MSG_FAIL);
                assert (common_msg);
                common_msg_set_errmsg (common_msg,
                    "Processing of messages with ID = '%d' "
                    "not implemented at the moment.", id);
                return_msg = common_msg_encode (&common_msg);
                assert (return_msg);
                assert (common_msg == NULL);
                assert (is_common_msg (return_msg));
                break;
            }
            case ASSET_MSG_GET_LOCATION_FROM:
            {
                return_msg =  get_return_topology_from (database_url, message);
                assert (return_msg);
                break;
            }
            case ASSET_MSG_GET_LOCATION_TO:
            {
                return_msg = get_return_topology_to (database_url, message);
                assert (return_msg);
                break;
            }
            case ASSET_MSG_GET_POWER_FROM:
            {
                return_msg = get_return_power_topology_from (database_url,
                                                            message);
                assert (return_msg);
                break;
            }
            case ASSET_MSG_GET_POWER_TO:
            {
                return_msg = get_return_power_topology_to (database_url, 
                                                            message);
                assert (return_msg);
                break;
            }
            case ASSET_MSG_GET_POWER_GROUP:
            {
                return_msg = get_return_power_topology_group (database_url, 
                                                            message);
                assert (return_msg);
                break;
            }
            case ASSET_MSG_GET_POWER_DATACENTER:
            {
                return_msg = get_return_power_topology_datacenter 
                                                    (database_url, message);
                assert (return_msg);
                break;
            }
            default:
            {
                log_warning ("Unexpected message type received. "
                        "Message ID: '%d'\n", id);
                
                _scoped_common_msg_t *common_msg = common_msg_new (COMMON_MSG_FAIL);
                assert (common_msg);
                common_msg_set_errmsg (common_msg,
                                       "Unexpected message type received. " 
                                       "Message ID: '%d'",  id);
                return_msg = common_msg_encode (&common_msg);
                assert (return_msg);
                assert (common_msg == NULL);
                assert (is_common_msg (return_msg));
                break;
            }
        }

        asset_msg_destroy (message_p);
        assert (*message_p == NULL);
    } else {
        log_error ("Pointer to null pointer passed as second argument "
            "'_scoped_asset_msg_t **message_p'.");
        return_msg = common_msg_encode_fail (0, 0,"Invalid asset message: "
            "Pointer to null pointer passed as second argument.", NULL);
        assert (return_msg);
        assert (is_common_msg (return_msg));
    }
    assert (return_msg); // safeguard non-NULL return value
    log_close ();
    return return_msg;
}

int matryoshka2frame (zmsg_t **matryoshka, zframe_t **frame )
{
    assert ( matryoshka );
    if ( *matryoshka ) {
        byte *buffer;
        size_t zmsg_size = zmsg_encode (*matryoshka, &buffer);

        // double check size
        // TODO after some time, remove the redundant check
        size_t check_size = 0;
        _scoped_zframe_t *tmp_frame = zmsg_first (*matryoshka);
        while ( tmp_frame ) {
            size_t tmp_frame_size = zframe_size (tmp_frame);
            if ( tmp_frame_size < 255 )
                check_size += tmp_frame_size + 1;
            else
                check_size += tmp_frame_size + 1 + 4;
            tmp_frame = zmsg_next (*matryoshka);
        }
        assert ( check_size == zmsg_size );

        zframe_t *ret_frame = zframe_new (buffer, zmsg_size);
        assert ( ret_frame );

        zmsg_destroy (matryoshka);
        assert ( *matryoshka == NULL );

        *frame = ret_frame;
        return 0;
    }
    else {
        return -2;
    }
}

size_t my_size(zframe_t* frame)
{
    if ( frame == NULL )
        return 0;
    else
        return zframe_size (frame);
}

zmsg_t* select_group_elements(
            const char*     url             , a_elmnt_id_t    element_id, 
            a_elmnt_tp_id_t element_type_id , const char*     group_name, 
            const char*     dtype_name      , a_elmnt_tp_id_t filtertype
        )
{
    assert ( element_id );  // id of the group should be specified
    assert ( element_type_id ); // type_id of the group
    assert ( ( filtertype >= asset_type::GROUP ) && ( filtertype <= 7 ) ); 
    // it can be only 1,2,3,4,5,6.7. 7 means - take all

    log_info ("start");
    log_debug ("element_id = %" PRIu32, element_id);
    log_debug ("filter_type = %" PRIu16, filtertype);
 
    try{
        tntdb::Connection conn = tntdb::connectCached(url); 
        tntdb::Statement st = conn.prepareCached(
                " SELECT"
                "   v.id_asset_element,"
                "   v1.name,"
                "   v1.id_type AS id_asset_element_type,"
                "   v3.name AS dtype_name,"
                "   v2.name AS name_asset_element_type"
                " FROM    t_bios_asset_group_relation v"
                "   INNER JOIN t_bios_asset_element v1"
                "       ON (v.id_asset_element = v1.id_asset_element )"
                "   INNER JOIN t_bios_asset_element_type v2"
                "       ON (v1.id_type = v2.id_asset_element_type)"
                "   LEFT JOIN v_bios_asset_device v3"
                "       ON v3.id_asset_element = v1.id_asset_element"
                "       WHERE v.id_asset_group = :elementid"
            );
            
        // Could return more than one row
        tntdb::Result result = st.set("elementid", element_id).
                                  select();
        
        log_debug("rows selected %u", result.size());
        int i = 0;
        int rv = 0;

        _scoped_zmsg_t* dcss     = zmsg_new();
        _scoped_zmsg_t* roomss   = zmsg_new();
        _scoped_zmsg_t* rowss    = zmsg_new();
        _scoped_zmsg_t* rackss   = zmsg_new();
        _scoped_zmsg_t* devicess = zmsg_new();

        for ( auto &row: result )
        {
            i++;
            a_elmnt_id_t id = 0;
            row[0].get(id);
            assert ( id );      // required field, otherwise db is corrupted
    
            std::string name = "";
            row[1].get(name);
            assert ( !name.empty() ); // otherwise db is corrupted

            a_elmnt_tp_id_t id_type = 0;
            row[2].get(id_type);
            assert ( id_type );
            
            std::string dtype_name = "";
            row[3].get(dtype_name);
            
            log_debug ("for");
            log_debug ("i = %d", i);
            log_debug ("id = %" PRIu32, id);
            log_debug ("name = %s", name.c_str());
            log_debug ("id_type = %" PRIu16, id_type);
            log_debug ("dtype_name = %s", dtype_name.c_str());

            // we are interested in this element if we are interested in 
            // all elements ( filtertype == 7) or if this element has 
            // exactly the type of the filter (filtertype == id_type)
            // or we are interested in groups details 
            // ( filtertype == asset_type::GROUP )
            if ( ( filtertype == 7 ) || ( filtertype == asset_type::GROUP )
                    || ( filtertype == id_type ) )
            {
                // dcs, rooms, rows, racks, devices, groups are NULL
                // because we provide only first layer of inclusion
                _scoped_zmsg_t* el = asset_msg_encode_return_location_from 
                              (id, id_type, name.c_str(), dtype_name.c_str(), 
                               NULL, NULL, NULL, NULL, NULL, NULL);
                assert ( el );
        
                // we are interested in this element
                log_debug ("created msg el for i = %d", i);

                // put elements into the bins by its asset_element_type_id
                if ( id_type == asset_type::DATACENTER )
                    rv = zmsg_addmsg (dcss, &el);
                else if ( id_type == asset_type::ROOM )
                    rv = zmsg_addmsg (roomss, &el);
                else if ( id_type == asset_type::ROW )
                    rv = zmsg_addmsg (rowss, &el);
                else if ( id_type == asset_type::RACK )
                    rv = zmsg_addmsg (rackss, &el);
                else if ( id_type == asset_type::DEVICE )
                    rv = zmsg_addmsg (devicess, &el);
                assert ( rv != -1 );
                assert ( el == NULL );                 
                // group of groups is not allowed 
            } // end of if interested in element
        }// end for
        
        _scoped_zframe_t* dcs     = NULL;
        _scoped_zframe_t* rooms   = NULL;
        _scoped_zframe_t* rows    = NULL;
        _scoped_zframe_t* racks   = NULL;
        _scoped_zframe_t* devices = NULL;
        
        // transform bins to the frames
        rv = matryoshka2frame (&dcss, &dcs);
        assert ( rv == 0 );
        rv = matryoshka2frame (&roomss, &rooms);
        assert ( rv == 0 );
        rv = matryoshka2frame (&rowss, &rows);
        assert ( rv == 0 );
        rv = matryoshka2frame (&rackss, &racks);
        assert ( rv == 0 );
        rv = matryoshka2frame (&devicess, &devices);
        assert ( rv == 0 );

        // generate message for the group with filled elements
        zmsg_t* el = asset_msg_encode_return_location_from 
                      (element_id, element_type_id, group_name, dtype_name, 
                       dcs, rooms, rows, racks, devices, NULL);
        assert ( el );
        log_info ("end");
        return el;
    }
    catch (const std::exception &e) {
        log_warning ("abort with err = '%s'", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
    }
}

zframe_t* select_childs(
    const char*     url             , a_elmnt_id_t element_id,
    a_elmnt_tp_id_t element_type_id , a_elmnt_tp_id_t child_type_id,
    bool            is_recursive    , uint32_t current_depth,
    a_elmnt_tp_id_t     filtertype)
{
    assert ( child_type_id );   // is required
    assert ( ( filtertype >= asset_type::GROUP ) && ( filtertype <= 7 ) ); 
    // it can be only 1,2,3,4,5,6.7. 7 means - take all

    log_info ("start select_childs");
    log_debug ("depth = %" PRIu32, current_depth);
    log_debug ("element_id = %" PRIu32, element_id);
    log_debug ("element_type_id = %" PRIu16, element_type_id);
    log_debug ("child_type_id = %" PRIu16, child_type_id);

    try{
        tntdb::Connection conn = tntdb::connectCached(url); 
        tntdb::Statement st;
        tntdb::Result result; 
        if ( element_id != 0 )
        {
            // for the groups other select is needed
            // because type of the group should be selected
            if ( child_type_id == asset_type::GROUP )
            {
                st = conn.prepareCached(
                    " SELECT"
                    "    v.id, v.name, v.id_type, v1.value as dtype_name"
                    " FROM v_bios_asset_element v"
                    "    INNER JOIN t_bios_asset_ext_attributes v1"
                    "      ON ( v.id = v1.id_asset_element AND"
                    "           v1.keytag = 'type')"
                    " WHERE v.id_parent = :elementid AND"
                    "       v.id_parent_type = :elementtypeid AND"
                    "       v.id_type = :childtypeid"
                );
            }
            else
            {
                st = conn.prepareCached(
                    " SELECT"
                    "    v.id, v.name, v.id_type, v1.name as dtype_name"
                    " FROM v_bios_asset_element v"
                    "    LEFT JOIN v_bios_asset_device v1"
                    "      ON (v.id = v1.id_asset_element)"
                    " WHERE v.id_parent = :elementid AND"
                    "       v.id_parent_type = :elementtypeid AND"
                    "       v.id_type = :childtypeid"
                );
            }
            // Could return more than one row
            result = st.set("elementid", element_id).
                        set("elementtypeid", element_type_id).
                        set("childtypeid", child_type_id).
                        select();
        }
        else
        {
            // for the groups, other select is needed
            // because type of the group should be selected
            if ( child_type_id == asset_type::GROUP )
            {
                st = conn.prepareCached(
                    " SELECT"
                    "    v.id, v.name, v.id_type, v1.value as dtype_name"
                    " FROM v_bios_asset_element v"
                    "   INNER JOIN t_bios_asset_ext_attributes v1"
                    "      ON ( v.id = v1.id_asset_element AND"
                    "           v1.keytag = 'type')"
                    " WHERE v.id_parent is NULL  AND"
                    "       v.id_type = :childtypeid"
                );
            }
            else
            {
                st = conn.prepareCached(
                    " SELECT"
                    "    v.id, v.name, v.id_type, v1.name as dtype_name"
                    " FROM v_bios_asset_element v"
                    "    LEFT JOIN v_bios_asset_device v1"
                    "      ON (v.id = v1.id_asset_element)"
                    " WHERE v.id_parent is NULL  AND "
                    "       v.id_type = :childtypeid"
                );
            }
            // Could return more than one row
            result = st.set("childtypeid", child_type_id).
                        select();
        }
        log_debug("rows selected %u", result.size());
        int rv = 0;
        _scoped_zmsg_t* ret = zmsg_new();
        int i = 0;
        for ( auto &row: result )
        {
            i++;
            uint32_t id = 0;
            row[0].get(id);
            assert ( id );
    
            std::string name = "";
            row[1].get(name);
            assert ( !name.empty() );

            uint16_t id_type = 0;
            row[2].get(id_type);
            assert ( id_type );
            
            std::string dtype_name = "";
            row[3].get(dtype_name);
            
            log_debug ("for");
            log_debug ("i = %d", i);
            log_debug ("id = %" PRIu32, id);
            log_debug ("name = %s", name.c_str());
            log_debug ("id_type = %" PRIu16, id_type);
            log_debug ("dtype_name = %s", dtype_name.c_str());

            _scoped_zframe_t* dcs     = NULL;
            _scoped_zframe_t* rooms   = NULL;
            _scoped_zframe_t* rows    = NULL;
            _scoped_zframe_t* racks   = NULL;
            _scoped_zframe_t* devices = NULL;
            _scoped_zmsg_t*   grp     = NULL;
            
            // Select childs only if it is not a leaf (is not a device), 
            // it is recursive search, and we didn't achive max 
            // recursion depth
            if (    ( is_recursive ) && 
                    ( child_type_id != asset_type::DEVICE ) && 
                    ( current_depth <= MAX_RECURSION_DEPTH ) )
            {
                // There is no need to select datacenters, because 
                // they could be seleted only for grops, but for groups
                // there is a special processing
                
                // Select rooms only for datacenters
                // and TODO filter
                if (    ( child_type_id == asset_type::DATACENTER ) && 
                        ( 3 <= filtertype ) )
                {
                    log_info ("start select_rooms");
                    rooms = select_childs (url, id, child_type_id, 
                                asset_type::ROOM, is_recursive, 
                                current_depth + 1, filtertype);
                    log_info ("end select_rooms");
                }

                // Select rows only for datacenters and rooms
                // and TODO filter
                if ( (  ( child_type_id == asset_type::DATACENTER ) ||
                        ( child_type_id == asset_type::ROOM ) )     && 
                     ( 4 <= filtertype ) )
                {
                    log_info ("start select_rows");
                    rows  = select_childs (url, id, child_type_id,
                                asset_type::ROW, is_recursive, 
                                current_depth + 1, filtertype);
                    log_info ("end select_rows");
                }
                
                // Select racks only for datacenters, rooms, rows
                // and TODO filter
                if ( (  ( child_type_id == asset_type::DATACENTER)  ||
                        ( child_type_id == asset_type::ROOM )       ||
                        ( child_type_id == asset_type::ROW ) )     && 
                     ( 5 <= filtertype ) )
                {
                    log_info ("start select_racks");
                    racks   = select_childs (url, id, child_type_id, 
                                asset_type::RACK, is_recursive, 
                                current_depth + 1, filtertype);
                    log_info ("end select_racks");
                }
                
                // Select devices only for datacenters, rooms, rows, racks
                // and TODO filter
                if ( (  ( child_type_id == asset_type::DATACENTER)  ||
                        ( child_type_id == asset_type::ROOM )       ||
                        ( child_type_id == asset_type::ROW )        ||
                        ( child_type_id == asset_type::RACK ) )     &&
                     ( 6 <= filtertype ) )
                {
                    log_info ("start select_devices");
                    devices = select_childs (url, id, child_type_id,
                                asset_type::DEVICE, is_recursive, 
                                current_depth + 1, filtertype);
                    log_info ("end select_devices");
                }
                
                // TODO filter
                // if it is a group, then do a special processing
                if (    ( child_type_id == asset_type::GROUP) && 
                        (   ( asset_type::GROUP == filtertype ) || 
                            ( filtertype == 7 ) 
                        ) )
                {
                    log_info ("start select elements of the grp");
                    grp = select_group_elements (url, id, asset_type::GROUP, 
                                name.c_str(), dtype_name.c_str(), filtertype);
                    log_info ("end select elements of the grp");
                }
            }
            // all sub elements selected

            // add this asset_element to return result
            // if   selecting ALL or
            //      for this element where selected sub elements or
            //      the type of the element is a filter type
            if ( !( ( filtertype < 7 ) &&
                    ( ( my_size(dcs) == 0 ) && ( my_size(rooms) == 0 ) 
                        && ( my_size(rows) == 0 ) && ( my_size(racks) == 0 ) 
                        && ( my_size(devices) == 0 )  
                    ) &&
                    ( child_type_id != filtertype ) 
                  )
               )
            {
                
                _scoped_zmsg_t* el;
                if (    ( child_type_id == asset_type::GROUP ) && 
                        ( is_recursive ) )
                    el = zmsg_dup (grp);   // because of the special group processing
                else 
                    el = asset_msg_encode_return_location_from 
                                (id, id_type, name.c_str(), 
                                 dtype_name.c_str(), dcs, rooms, 
                                 rows, racks, devices, NULL);
                assert ( el );
                log_debug ("created msg el for i = %d",i);
                rv = zmsg_addmsg ( ret, &el);
                assert ( rv != -1 );
                assert ( el == NULL );
                zframe_destroy (&dcs);
                zframe_destroy (&rooms);
                zframe_destroy (&rows);
                zframe_destroy (&racks);
                zframe_destroy (&devices);
            }
        }// end for
        zframe_t* res = NULL;
        rv = matryoshka2frame (&ret, &res);
        assert ( rv == 0 );
        log_info ("end");
        return res;
    }
    catch (const std::exception &e) {
        log_warning ("abort with err = '%s'", e.what());
        return NULL;
    }
}

zmsg_t* get_return_topology_from(const char* url, asset_msg_t* getmsg)
{
    assert ( getmsg );
    assert ( url );
    assert ( asset_msg_id (getmsg) == ASSET_MSG_GET_LOCATION_FROM );
    log_info ("start");

    uint32_t element_id   = asset_msg_element_id  (getmsg);
    uint8_t  filter_type  = asset_msg_filter_type (getmsg);
    bool     is_recursive = false;
    uint16_t type_id      = 0;

    // element_id == 0 => we are looking for unlocated elements;
    // recursive := false; 
    if (element_id != 0) {
        is_recursive = asset_msg_recursive (getmsg);
    } 
    
    _scoped_zframe_t* dcs     = NULL;
    _scoped_zframe_t* rooms   = NULL;
    _scoped_zframe_t* rows    = NULL;
    _scoped_zframe_t* racks   = NULL;
    _scoped_zframe_t* devices = NULL;
    _scoped_zframe_t* grps    = NULL;
    
    std::string name = "";
    std::string dtype_name = "";

    // select additional information about starting device
    if ( element_id != 0 )
    {
        // if looking for a lockated elements
        try{
            tntdb::Connection conn = tntdb::connectCached(url);
            tntdb::Statement st = conn.prepareCached(
                " SELECT"
                "    v.name, v1.name as dtype_name, v.id_type"
                " FROM v_bios_asset_element v"
                "    LEFT JOIN v_bios_asset_device v1"
                "      ON (v.id = v1.id_asset_element)"
                " WHERE v.id = :id"
            );
    
            tntdb::Row row = st.set("id", element_id).
                                selectRow();
            
            row[0].get(name);
            assert ( !name.empty() );

            row[1].get(dtype_name);
            // assert (dtype_name != "" if type_id == DEVICE
            row[2].get(type_id);
            assert ( type_id );
   
        }
        catch (const tntdb::NotFound &e) {
            // element with specified id was not found
            log_warning ("abort select element with err = '%s'", e.what());
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_NOTFOUND, 
                                                        e.what(), NULL);
        }
        catch (const std::exception &e) {
            // internal error in database
            log_warning ("abort select element with err = '%s'", e.what());
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
        }

        if ( type_id == asset_type::GROUP ) 
        {
            try{
                tntdb::Connection conn = tntdb::connectCached(url);
                tntdb::Statement st = conn.prepareCached(
                    " SELECT"
                    "    v.value"
                    " FROM"
                    "    t_bios_asset_ext_attributes v"
                    " WHERE v.id_asset_element = :elementid AND "
                    "       v.keytag = 'type'"
                );
                tntdb::Row row = st.set("elementid", element_id).

                                    selectRow();
    
                log_debug("element_id = %" PRIu32, element_id);
                row[0].get(dtype_name);
                assert ( !dtype_name.empty() ) ; 
            }
            catch (const tntdb::NotFound &e) {
                // atribute type for the group was not specified, 
                // but it is a mandatory
                log_warning ("abort type for the group was not specified"
                                " err = '%s'\n", e.what());
                return common_msg_encode_fail (BIOS_ERROR_DB, 
                        DB_ERROR_DBCORRUPTED, e.what(), NULL);
            }
            catch (const std::exception &e) {
                // internal error in database
                log_warning ("abort select element with err = '%s'", e.what());
                return common_msg_encode_fail (BIOS_ERROR_DB, 
                        DB_ERROR_INTERNAL, e.what(), NULL);
            }

        }

    }

    // Select sub elements by types

    // Select datacenters
    // ACE: 11.12.14 according rfc and the logic, there is no need to 
    // select datacenters

    // Select rooms
    // only for datacenters according filter
    // TODO filter
    if ( ( ( type_id == asset_type::DATACENTER ) || 
           ( element_id == 0 ) ) &&
         ( 3 <= filter_type ) )
        
    {
        log_info ("start select_rooms");
        rooms = select_childs (url, element_id, type_id, asset_type::ROOM,
                        is_recursive, 1, filter_type);
        if ( rooms == NULL )
        {
            zframe_destroy (&dcs);
            log_warning ("end abnormal");
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                    "rooms error", NULL);
        }
        log_info ("end select_rooms");
    }
    // Select rows
    // only for rooms, datacenters, unlockated
    // TODO filter
    if ( ( ( type_id == asset_type::DATACENTER )  ||
           ( type_id == asset_type::ROOM )       || 
           ( element_id == 0 ) ) &&
         ( 4 <= filter_type ) )
    {
        log_info ("start select_rows");
        rows = select_childs (url, element_id, type_id, asset_type::ROW,
                        is_recursive, 1, filter_type);
        if ( rows == NULL )
        {
            zframe_destroy (&dcs);
            zframe_destroy (&rooms);
            log_warning("end abnormal");
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                    "rows error", NULL);
        }
        log_info ("end select_rows");
    }
    // Select racks
    // only for rooms, datacenters, rows, unlockated
    // TODO filter
    if ( ( ( type_id == asset_type::DATACENTER)  ||
           ( type_id == asset_type::ROOM )       ||
           ( type_id == asset_type::ROW )        || 
           ( element_id == 0 ) ) &&
         ( 5 <= filter_type ) )
    {
        log_info ("start select_racks");
        racks = select_childs (url, element_id, type_id, asset_type::RACK,
                        is_recursive, 1, filter_type);
        if ( racks == NULL )
        {
            zframe_destroy (&dcs);
            zframe_destroy (&rooms);
            zframe_destroy (&rows);
            log_warning ("end abnormal");
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                    "racks error", NULL);
        }
        log_info ("end select_racks");
    }
    // Select devices
    // only for rooms, datacenters, rows, racks, unlockated
    // TODO filter
    if ( ( ( type_id == asset_type::DATACENTER)  ||
           ( type_id == asset_type::ROOM )       ||
           ( type_id == asset_type::ROW )        ||
           ( type_id == asset_type::RACK )     ||
           ( element_id == 0 )  ) &&
         ( 6 <= filter_type ) )
    {
        log_info ("start select_devices");
        devices = select_childs (url, element_id, type_id, asset_type::DEVICE,
                        is_recursive, 1, filter_type);
        if ( devices == NULL )
        {
            zframe_destroy (&dcs);
            zframe_destroy (&rooms);
            zframe_destroy (&rows);
            zframe_destroy (&racks);
            log_warning ("end abnormal");
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                    "devices error", NULL);
        }
        log_info ("end select_devices");
    }
    // Select groups
    // Groups can be selected
    //      - only for datacenter (if selecting all childs  or only groups).
    //      - unlockated.
    // TODO filter
    if  ( ( ( type_id == asset_type::DATACENTER ) && 
            ( ( filter_type == asset_type::GROUP ) || 
              ( filter_type == 7 ) ) ) ||
          ( element_id == 0 ) )
    {
        log_info ("start select_grps");
        grps = select_childs (url, element_id, type_id, asset_type::GROUP,
                        is_recursive, 1, filter_type);
        if ( grps == NULL )
        {
            zframe_destroy (&dcs);
            zframe_destroy (&rooms);
            zframe_destroy (&rows);
            zframe_destroy (&racks);
            zframe_destroy (&devices);
            log_warning ("end abnormal");
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                    "groups error", NULL);
        }
        log_info ("end select_grps");
    }
    
    zmsg_t* el = NULL; 
    log_info ("creating return element");
    if ( type_id == asset_type::GROUP )
    {
        el = select_group_elements (url, element_id, type_id, name.c_str(), 
                                    dtype_name.c_str(), filter_type);
    }
    else
    {
        el = asset_msg_encode_return_location_from 
                    (element_id, type_id, name.c_str(), 
                     dtype_name.c_str(), dcs, rooms, rows, 
                     racks, devices, grps);
    }
    log_info ("end normal");
    return el;
}

bool compare_start_element (asset_msg_t* rmsg, uint32_t id, uint8_t id_type,
                            const char* name, const char* dtype_name)
{
    if ( ( asset_msg_id (rmsg) != ASSET_MSG_RETURN_LOCATION_FROM )  &&
             ( asset_msg_id (rmsg) != ASSET_MSG_RETURN_LOCATION_TO ) )
        return false;
    else if (   ( asset_msg_element_id (rmsg) == id ) && 
                ( asset_msg_type (rmsg) == id_type )  && 
                ( !strcmp (asset_msg_name (rmsg), name) ) && 
                ( !strcmp (asset_msg_type_name (rmsg), dtype_name) ) 
            )
        return true;
    else
        return false;
}

edge_lf print_frame_to_edges (zframe_t* frame, a_elmnt_id_t parent_id, 
                a_elmnt_tp_id_t type, std::string name, std::string dtype_name)
{    
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    edge_lf result, result1;

    _scoped_zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
     
    _scoped_zmsg_t* pop = NULL;
    while ( ( pop = zmsg_popmsg (zmsg) ) != NULL )
    { 
        _scoped_asset_msg_t* item = asset_msg_decode (&pop); // zmsg_t is freed
        assert ( item );
//        asset_msg_print (item);
        
        result.insert(std::make_tuple (
                    asset_msg_element_id(item), 
                    asset_msg_type(item), 
                    asset_msg_name(item), 
                    asset_msg_type_name(item), 
                    parent_id, type, name, dtype_name) ); 
        log_debug ("parent_id = %" PRIu32, parent_id );
        
        zframe_t* fr = asset_msg_dcs (item);
        result1 = print_frame_to_edges (fr, asset_msg_element_id (item), 
                                            asset_msg_type(item), 
                                            asset_msg_name(item),
                                            asset_msg_type_name(item));
        result.insert(result1.begin(), result1.end());
        
        fr = asset_msg_rooms (item);
        result1 = print_frame_to_edges (fr, asset_msg_element_id (item),
                                            asset_msg_type(item), 
                                            asset_msg_name(item), 
                                            asset_msg_type_name(item));
        result.insert(result1.begin(), result1.end());
        
        fr = asset_msg_rows (item);
        result1 = print_frame_to_edges (fr, asset_msg_element_id (item),
                                            asset_msg_type(item), 
                                            asset_msg_name(item), 
                                            asset_msg_type_name(item));
        result.insert(result1.begin(), result1.end());
        
        fr = asset_msg_racks(item);
        result1 = print_frame_to_edges (fr, asset_msg_element_id (item),
                                            asset_msg_type(item), 
                                            asset_msg_name(item), 
                                            asset_msg_type_name(item));
        result.insert(result1.begin(), result1.end());
        
        fr = asset_msg_devices (item);
        result1 = print_frame_to_edges (fr, asset_msg_element_id (item),
                                            asset_msg_type(item), 
                                            asset_msg_name(item), 
                                            asset_msg_type_name(item));
        result.insert(result1.begin(), result1.end());
        
        fr = asset_msg_grps (item);
        result1 = print_frame_to_edges (fr, asset_msg_element_id (item),
                                            asset_msg_type(item), 
                                            asset_msg_name(item), 
                                            asset_msg_type_name(item));
        result.insert(result1.begin(), result1.end());
        
        asset_msg_destroy (&item);
        assert ( pop == NULL );
   }
   zmsg_destroy (&zmsg);
   return result;
}

void print_frame (zframe_t* frame, a_elmnt_id_t parent_id)
{    
    byte* buffer = zframe_data (frame);
    assert ( buffer );

    _scoped_zmsg_t* zmsg = zmsg_decode (buffer, zframe_size (frame));
    assert ( zmsg );
     
    _scoped_zmsg_t* pop = NULL;
    while ( ( pop = zmsg_popmsg (zmsg) ) != NULL )
    { 
        _scoped_asset_msg_t* item = asset_msg_decode (&pop); // zmsg_t is freed
        assert ( item );
        asset_msg_print (item);
        log_debug("parent_id = %" PRIu32, parent_id );
        
        _scoped_zframe_t* fr = asset_msg_dcs (item);
        print_frame (fr, asset_msg_element_id (item));
        fr = asset_msg_rooms (item);
        print_frame (fr, asset_msg_element_id (item));
        fr = asset_msg_rows (item);
        print_frame (fr, asset_msg_element_id (item));
        fr = asset_msg_racks(item);
        print_frame (fr, asset_msg_element_id (item));
        fr = asset_msg_devices (item);
        print_frame (fr, asset_msg_element_id (item));
        fr = asset_msg_grps (item);
        print_frame (fr, asset_msg_element_id (item));
        asset_msg_destroy (&item);
        assert ( pop == NULL );
    }
   zmsg_destroy (&zmsg);
}

zmsg_t* select_parents (const char* url, a_elmnt_id_t element_id, 
                        a_elmnt_tp_id_t element_type_id)
{
    assert ( element_id );      // is required
    assert ( element_type_id ); // is required

    log_info ("start");
    log_debug ("element_id = %" PRIu32, element_id);
    log_debug ("element_type_id = %" PRIu16, element_type_id);

    try{
        tntdb::Connection conn = tntdb::connectCached(url); 
        tntdb::Statement st;
        
        // for the groups, other select is needed
        // because type of the group should be selected
        if ( element_type_id != asset_type::GROUP )
        {
            st = conn.prepareCached(
                  " SELECT"
                  "     v.id_parent, v.id_parent_type,v.name,"
                  "     v1.name as dtype_name"
                  " FROM"
                  "     v_bios_asset_element v"
                  "     LEFT JOIN v_bios_asset_device v1"
                  "      ON (v.id = v1.id_asset_element)"
                  " WHERE v.id = :elementid AND"
                  "       v.id_type = :elementtypeid"
            );
        }
        else
        {
            st = conn.prepareCached(
                  " SELECT"
                  "     v.id_parent, v.id_parent_type,v.name,"
                  "     v1.value as dtype_name"
                  " FROM"
                  "     v_bios_asset_element v"
                  "     INNER JOIN t_bios_asset_ext_attributes v1"
                  "      ON (v.id = v1.id_asset_element AND"
                  "          v1.keytag = 'type')"
                  " WHERE v.id = :elementid AND "
                  "       v.id_type = :elementtypeid"
            );
        }

        // Could return one row or nothing
        tntdb::Row row = st.set("elementid", element_id).
                            set("elementtypeid", element_type_id).
                            selectRow();
    
        uint32_t parent_id = 0;
        uint16_t parent_type_id = 0;

        std::string dtype_name = ""; 
        std::string name = "";
        row[0].get(parent_id);
        row[1].get(parent_type_id);
        row[2].get(name);
        row[3].get(dtype_name);

        log_debug("rows selected %u, parent_id = %" PRIu32 ", "
                "parent_type_id = %" PRIu16, 1,  parent_id, parent_type_id);
        
        if ( parent_id != 0 )
        {
            _scoped_zmsg_t* parent = select_parents (url, parent_id, parent_type_id);
            if ( is_asset_msg (parent) ) {
                return asset_msg_encode_return_location_to (element_id, 
                            element_type_id, name.c_str(), 
                            dtype_name.c_str(), parent);
            }
            else if ( is_common_msg (parent) )
                return zmsg_dup (parent);
            else
                return common_msg_encode_fail (BIOS_ERROR_DB, 
                        DB_ERROR_INTERNAL, "UNSUPPORTED RETURN MESSAGE TYPE", 
                        NULL);
        }
        else
        {
            log_info ("but this element has no parent");
            return asset_msg_encode_return_location_to (element_id, 
                    element_type_id, name.c_str(), dtype_name.c_str(), 
                    zmsg_new());
        }
    }
    catch (const tntdb::NotFound &e) {
        // element with specified type was not found
        log_warning ("abort with err = '%s'", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_NOTFOUND, 
                                                        e.what(), NULL);
    }
    catch (const std::exception &e) {
        // internal error in database
        log_warning ("abort with err = '%s'", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
    }
}

zmsg_t* get_return_topology_to(const char* url, asset_msg_t* getmsg)
{
    assert ( getmsg );
    assert ( url );
    assert ( asset_msg_id (getmsg) == ASSET_MSG_GET_LOCATION_TO );
    log_info ("start");
    a_elmnt_id_t element_id = asset_msg_element_id (getmsg);
    log_debug("element_id=%" PRIu32, element_id);
    a_elmnt_tp_id_t type_id = 0;    
 
    // select additional information about starting device
    try{
        tntdb::Connection conn = tntdb::connectCached(url); 
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "    v.id_type"
            " FROM v_bios_asset_element v"
            " WHERE v.id = :id"
        );

        tntdb::Row row = st.set("id", element_id).
                            selectRow();
        
        row[0].get(type_id);
        assert ( type_id );
    }
    catch (const tntdb::NotFound &e) {
        // element with specified id was not found
        log_warning ("abort select element with err = '%s'", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_NOTFOUND, 
                                                    e.what(), NULL);
    }
    catch (const std::exception &e) {
        // internal error in database
        log_warning ("abort select element with err = '%s'", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                    e.what(), NULL);
    }
    
    log_debug("type_id=%" PRIu16, type_id);
    zmsg_t* result = select_parents (url, element_id, type_id);

    log_info ("end");
    return result;
}

std::tuple <std::string, std::string, a_elmnt_tp_id_t>
    select_add_device_info  ( const char* url, 
                              a_elmnt_id_t asset_element_id)
{
    // select information about specidied asset element
    tntdb::Connection conn = tntdb::connectCached(url);

    tntdb::Statement st = conn.prepareCached(
        " SELECT"
        "   v.name, v1.name as type_name, v1.id_asset_device_type"
        " FROM"
        "   v_bios_asset_element v"
        " LEFT JOIN  v_bios_asset_device v1"
        "   ON ( v1.id_asset_element = v.id )"
        " WHERE v.id = :id"
    );
    tntdb::Row row = st.set("id", asset_element_id).
                        selectRow();
    
    // asset element name, required
    std::string element_name = "";
    row[0].get(element_name);
    assert ( !element_name.empty() ); // database is corrupted
    log_debug ("selected device name = %s", element_name.c_str());
     
    // device type name, would be NULL if it is not a device
    std::string device_type_name = "";
    row[1].get(device_type_name);
    log_debug ("selected device type name = %s", device_type_name.c_str());
    
    // device type id, would be 0 if it is not a device
    a_dvc_tp_id_t device_type_id = 0;
    row[2].get(device_type_id);
    log_debug ("selected device type id = %" PRIu16, device_type_id);
    return std::make_tuple (element_name, device_type_name, device_type_id);
}

zmsg_t* convert_powerchain_devices2matryoshka (
                    std::set < device_info_t > const &devices)
{
    // tuple: ( id,  device_name, device_type_name )
    // encode: id, device_type_name, device_name
    zmsg_t* ret = zmsg_new();
    for ( auto &adevice : devices )
    {
        _scoped_zmsg_t* el = asset_msg_encode_powerchain_device
                    (std::get<0>(adevice), (std::get<2>(adevice)).c_str(), 
                    (std::get<1>(adevice)).c_str() );
        int rv = zmsg_addmsg (ret, &el);
        assert ( rv != -1 );
        assert ( el == NULL );
    }
    return ret;
}

zlist_t* convert_powerchain_powerlink2list (
                std::set < powerlink_info_t > const &powerlinks)
{
    zlist_t* powers = zlist_new();
    zlist_autofree (powers);
    for ( auto it = powerlinks.begin(); it != powerlinks.end(); ++it )
    {
        auto apowerlink = *it;

        zlist_push(powers, (char *)(std::get<1>(apowerlink) + ":" 
                        + std::to_string (std::get<0>(apowerlink)) + ":" 
                        + std::get<3>(apowerlink) + ":" 
                        + std::to_string (std::get<2>(apowerlink))).c_str());
    }
    return powers;
}

zmsg_t* generate_return_power (std::set < device_info_t >    const &devices, 
                               std::set < powerlink_info_t > const &powerlinks)
{
    _scoped_zmsg_t  *devices_msg = convert_powerchain_devices2matryoshka (devices);
    _scoped_zlist_t *powers_list = convert_powerchain_powerlink2list     (powerlinks);
    
    _scoped_zframe_t *devices_frame = NULL;
    int rv = matryoshka2frame (&devices_msg, &devices_frame);
    assert ( rv == 0 );
    zmsg_t *result = asset_msg_encode_return_power 
                                                (devices_frame, powers_list);
    zlist_destroy  (&powers_list);
    zframe_destroy (&devices_frame);
    return result;
}

zmsg_t* get_return_power_topology_from(const char* url, asset_msg_t* getmsg)
{
    assert ( getmsg );
    assert ( asset_msg_id (getmsg) == ASSET_MSG_GET_POWER_FROM );
    log_info ("start");
    a_elmnt_id_t     element_id = asset_msg_element_id  (getmsg);
    a_elmnt_tp_id_t  linktype   = INPUT_POWER_CHAIN;
    log_debug ("element_id = %" PRIu32, element_id);
    log_debug ("linktype_id = %" PRIu16, linktype);

    std::string device_name = "";
    std::string device_type_name = "";
    a_dvc_tp_id_t device_type_id = 0;

    // select information about the start device
    try{
        std::tuple <std::string, std::string, a_dvc_tp_id_t> additional_info = 
            select_add_device_info (url, element_id);
        device_name = std::get<0>(additional_info);
        device_type_name = std::get<1>(additional_info);
        device_type_id = std::get<2>(additional_info);
    }
    catch (const tntdb::NotFound &e) {
        // device with specified id was not found
        log_warning ("abort with err = '%s'", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_NOTFOUND, 
                                                        e.what(), NULL);
    }
    catch (const std::exception &e) {
        // internal error in database
        log_warning ("abort with err = '%s'", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
    }
    
    // check, if selected element is device
    if ( device_type_name.empty() )
    {   // then it is not a device
        log_warning ("abort with err = '%s %" PRIu32 " %s'", 
                        "specified element id =", element_id, 
                        " is not a device");
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_BADINPUT, 
                                        "specified element is not a device", 
                                        NULL);
    }

    // select powerlinks from start device, but only first level connections
    //      all powerlinks are included into "resultpowers"
    std::set< powerlink_info_t > resultpowers;
    //      all destination devices are included into "resultdevices"
    std::set< device_info_t > resultdevices;
    //      start device should be included also into the result set
    resultdevices.insert (
        std::make_tuple(element_id, device_name, 
                                        device_type_name, device_type_id));
    
    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "  v.id_asset_element_dest, v.src_out, v.dest_in, v.dest_name,"
            "  v.dest_type_name, v.dest_type_id"
            " FROM"
            "  v_bios_asset_link_topology v"
            " WHERE"
            "  v.id_asset_link_type = :idlinktype AND"
            "  v.id_asset_element_src = :id"
        );
        // can return more than one value
        tntdb::Result result = st.set("id", element_id).
                                  set("idlinktype", linktype).
                                  select();
        
        log_debug ("links selected: %u", result.size());
        
        // Go through the selected links
        for ( auto row: result )
        {
            // id_asset_element_dest, requiured
            a_elmnt_id_t id_asset_element_dest = 0;
            row[0].get(id_asset_element_dest);
            assert ( id_asset_element_dest );
   
            // src_out 
            std::string src_out = SRCOUT_DESTIN_IS_NULL;
            row[1].get(src_out);
            
            // dest_in
            std::string dest_in = SRCOUT_DESTIN_IS_NULL;
            row[2].get(dest_in);
            
            // device_name, required
            std::string device_name = "";
            row[3].get(device_name);
            assert ( !device_name.empty() );

            // device_type_name, requiured
            std::string device_type_name = "";
            row[4].get(device_type_name);
            assert ( !device_type_name.empty() );
            
            // id_asset_element_dest, requiured
            a_elmnt_id_t device_type_dest_id = 0;
            row[5].get(device_type_dest_id);
            assert ( device_type_dest_id );
   
            log_debug ("for");
            log_debug ("asset_element_id_src = %" PRIu32, element_id);
            log_debug ("asset_element_id_dest = %" PRIu32, 
                                                    id_asset_element_dest);
            log_debug ("src_out = %s", src_out.c_str());
            log_debug ("dest_in = %s", dest_in.c_str());
            log_debug ("device_name = %s", device_name.c_str());
            log_debug ("device_type_name = %s", device_type_name.c_str());
            log_debug ("dest_type_id = %" PRIu32, device_type_dest_id);

            resultpowers.insert  (std::make_tuple(
                    element_id, src_out, id_asset_element_dest, dest_in)); 
            resultdevices.insert (std::make_tuple(
                    id_asset_element_dest, device_name, 
                    device_type_name, device_type_dest_id));
        } // end for
    }
    catch (const std::exception &e) {
        // internal error in database
        log_warning ("abort with err = '%s'", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
    }


    zmsg_t* result = generate_return_power (resultdevices, resultpowers);
    log_info ("end normal");
    return result;
}

void print_frame_devices (zframe_t* frame)
{    
    byte* buffer = zframe_data (frame);
    assert ( buffer );

    _scoped_zmsg_t* zmsg = zmsg_decode (buffer, zframe_size (frame));
    assert ( zmsg );
     
    _scoped_zmsg_t* pop = NULL;
    while ( ( pop = zmsg_popmsg (zmsg) ) != NULL )
    { 
        _scoped_asset_msg_t* item = asset_msg_decode (&pop);
        assert ( item );
        asset_msg_print (item);
        asset_msg_destroy (&item);
        assert ( pop == NULL );
    }
   zmsg_destroy (&zmsg);
}

std::pair < std::set < device_info_t >, std::set < powerlink_info_t > > 
select_power_topology_to (const char* url, a_elmnt_id_t element_id, 
                          a_lnk_tp_id_t linktype, bool is_recursive)
{
    log_info ("start");
    std::string device_name = "";
    std::string device_type_name = "";
    a_dvc_tp_id_t device_type_id = 0;
    
    // select information about the start device
    try{
        std::tuple <std::string, std::string, a_dvc_tp_id_t> additional_info = 
            select_add_device_info (url, element_id);
        device_name = std::get<0>(additional_info);
        device_type_name = std::get<1>(additional_info);
        device_type_id = std::get<2>(additional_info);
    }
    catch (const tntdb::NotFound &e) {
        // device with specified id was not found
        throw bios::NotFound();
    }
    catch (const std::exception &e) {
        // internal error in database
        throw bios::InternalDBError(e.what());
    }
    
    // check, if selected element is a device
    if ( device_type_name.empty() )
        throw bios::ElementIsNotDevice(); // then it is not a device

    // devices that should be searched for powerlinks
    // (used for the elimination of duplicated devices and powerlinks)
    std::set< device_info_t > newdevices;
    // result set of found devices 
    std::set< device_info_t > resultdevices;
                                                      
    auto adevice = std::make_tuple(element_id, device_name, 
                                            device_type_name, device_type_id);
    // start device should be included also into the result set
    resultdevices.insert (adevice);
    newdevices.insert (adevice);

    // all powerlinks are included into "resultpowers"
    std::set< powerlink_info_t > resultpowers;

    // indicates if we have devices that were not processed yet
    bool ncontinue = true;
    while ( ncontinue )
    {
        // 1. current element id we are looking powerlinks from
        a_elmnt_id_t cur_element_id = std::get<0>(adevice);
        
        // 2. select and process process powerlinks
        try{
            tntdb::Connection conn = tntdb::connectCached(url);

            tntdb::Statement st = conn.prepareCached(
                " SELECT"
                "  v.id_asset_element_src, v.src_out, v.dest_in, v.src_name,"
                "  v.src_type_name, v.src_type_id "
                " FROM"
                "  v_bios_asset_link_topology v"
                " WHERE"
                "  v.id_asset_link_type = :idlinktype AND"
                "  v.id_asset_element_dest = :id"
            );
            
            // can return more than one value
            tntdb::Result result = st.set("id", cur_element_id).
                                      set("idlinktype", linktype).
                                      select();
            
            log_debug ("for element_id= %" PRIu32 " was %u "
                    "powerlinks selected", cur_element_id, result.size());
            
            // Go through the selected links
            for ( auto &row: result )
            {
                // id_asset_element_dest, requiured
                a_elmnt_id_t id_asset_element_src = 0;
                row[0].get(id_asset_element_src);
                assert ( id_asset_element_src );
       
                // src_out 
                std::string src_out = SRCOUT_DESTIN_IS_NULL;
                row[1].get(src_out);
                
                // dest_in
                std::string dest_in = SRCOUT_DESTIN_IS_NULL;
                row[2].get(dest_in);
                
                // device_name_src, required
                std::string device_name_src = "";
                row[3].get(device_name_src);
                assert ( !device_name_src.empty() );
    
                // device_type_name_src, requiured
                std::string device_type_name_src = "";
                row[4].get(device_type_name_src);
                assert ( !device_type_name_src.empty() );
                
                // device_type_src_id, required
                a_elmnt_id_t device_type_src_id = 0;
                row[5].get(device_type_src_id);
                assert ( device_type_src_id );
                
                log_debug ("for");
                log_debug ("asset_element_id_dest = %" PRIu32, cur_element_id);
                log_debug ("asset_element_id_src = %" PRIu32, 
                                                    id_asset_element_src);
                log_debug ("src_out = %s", src_out.c_str());
                log_debug ("dest_in = %s", dest_in.c_str());
                log_debug ("device_name_src = %s", device_name_src.c_str());
                log_debug ("device_type_name_src = %s", 
                                                device_type_name_src.c_str());
    
                resultpowers.insert  (std::make_tuple(
                    id_asset_element_src, src_out, cur_element_id, dest_in)); 
                // add new devices to process
                if ( is_recursive )
                    newdevices.insert (std::make_tuple(
                            id_asset_element_src, device_name_src, 
                            device_type_name_src, device_type_src_id));
                else
                    resultdevices.insert (std::make_tuple(
                            id_asset_element_src, device_name_src, 
                            device_type_name_src, device_type_src_id));
            } // end for
        }
        catch (const std::exception &e) {
            // internal error in database
            throw bios::InternalDBError(e.what());
        }

        // 3. find the next dest device 
        ncontinue = false;
        // go through all found devices
        // and select one that was not yet processed (is not in resultdevices)
        for ( auto it = newdevices.begin(); it != newdevices.end(); ++it )
        {
            adevice = *it;
            if ( resultdevices.count (adevice) == 1 )  // it could be 0 or 1
                continue;
            else
            {
                resultdevices.insert (adevice);
                ncontinue = true;
                break;
            }
        }
    }   // end of processing one of the new devices
    log_info ("end normal");
    return std::make_pair (resultdevices, resultpowers);
}

zmsg_t* get_return_power_topology_to (const char* url, asset_msg_t* getmsg)
{
    assert ( getmsg );
    assert ( asset_msg_id (getmsg) == ASSET_MSG_GET_POWER_TO );
    log_info ("start");
    a_elmnt_id_t element_id = asset_msg_element_id (getmsg);
    a_lnk_tp_id_t  linktype   = INPUT_POWER_CHAIN;
    
    std::pair <std::set<device_info_t>, std::set <powerlink_info_t>> topology;

    // Always do a recursive search
    try{
        topology = select_power_topology_to (url, element_id, linktype, true);
    }
    catch (const bios::NotFound &e) {
        // device with specified id was not found
        log_warning ("abort with err = '%s'", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_NOTFOUND, 
                                                        e.what(), NULL);
    }
    catch (const bios::InternalDBError &e) {
        // internal error in database
        log_warning ("abort with err = '%s'", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
    }
    catch (const bios::ElementIsNotDevice &e) {
        // specified element is not a device
        log_warning ("abort with err = '%s %" PRIu32 " %s'", 
                "specified element id =", element_id, " is not a device");
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_BADINPUT, 
                                                        e.what(), NULL);
    }
    catch (const std::exception &e) {
        // unexpected error
        log_warning ("abort with err = '%s'", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_UNKNOWN, 
                                                        e.what(), NULL);
    }

    zmsg_t* result = generate_return_power (topology.first, topology.second);
    
    log_info ("end normal");
    return result;
} 

zmsg_t* get_return_power_topology_group(const char* url, asset_msg_t* getmsg)
{
    assert ( getmsg );
    assert ( asset_msg_id (getmsg) == ASSET_MSG_GET_POWER_GROUP );
    log_info ("start");
    a_elmnt_id_t  element_id = asset_msg_element_id (getmsg);
    a_lnk_tp_id_t linktype   = INPUT_POWER_CHAIN;

    log_info ("start select powers"); 
    //  all powerlinks are included into "resultpowers"
    std::set< powerlink_info_t > resultpowers;
    try{
        tntdb::Connection conn = tntdb::connectCached(url);
        // v_bios_asset_link are only devices, 
        // so there is no need to add more constrains
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "   v.src_out, v.id_asset_element_src,"
            "   v.dest_in, v.id_asset_element_dest"
            " FROM"
            "   v_bios_asset_link v"
            " WHERE"
            "       v.id_asset_link_type = :linktypeid AND"
            "       v.id_asset_element_src IN ("
            "               SELECT" 
            "                   v1.id_asset_element"
            "               FROM"
            "                   v_bios_asset_group_relation v1"
            "               WHERE v1.id_asset_group = :groupid"
            "             )"
            "   AND"
            "       v.id_asset_element_dest IN ("
            "               SELECT"
            "                   v2.id_asset_element"
            "               FROM"
            "                   v_bios_asset_group_relation v2"
            "               WHERE v2.id_asset_group = :groupid"
            "             )"
        );
        // can return more than one row
        tntdb::Result result = st.set("groupid", element_id).
                                  set("linktypeid", linktype).
                                  select();
        log_debug("rows selected %u", result.size()); 

        // Go through the selected links
        for ( auto &row: result )
        {
            // src_out 
            std::string src_out = SRCOUT_DESTIN_IS_NULL;
            row[0].get(src_out);
            
            // id_asset_element_src, required
            a_elmnt_id_t id_asset_element_src = 0;
            row[1].get(id_asset_element_src);
            assert ( id_asset_element_src );
            
            // dest_in
            std::string dest_in = SRCOUT_DESTIN_IS_NULL;
            row[2].get(dest_in);
            
            // id_asset_element_dest, required
            a_elmnt_id_t id_asset_element_dest = 0;
            row[3].get(id_asset_element_dest);
            assert ( id_asset_element_dest );
            
            log_debug ("for");
            log_debug ("asset_element_id_src = %" PRIu32, 
                                                    id_asset_element_src);
            log_debug ("asset_element_id_dest = %" PRIu32, 
                                                    id_asset_element_dest);
            log_debug ("src_out = %s", src_out.c_str());
            log_debug ("dest_in = %s", dest_in.c_str());

            resultpowers.insert  (std::make_tuple(
              id_asset_element_src, src_out, id_asset_element_dest, dest_in)); 
        } // end for
    }
    catch (const std::exception &e) {
        // internal error in database
        log_warning ("abort with err = '%s'", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
    }
    log_info ("end select powers");
    
    log_info ("start select devices"); 
    // result set of found devices 
    std::set< device_info_t > resultdevices;
    try{
        tntdb::Connection conn = tntdb::connectCached(url);
        // select is done from pure t_bios_asset_element, 
        // because v_bios_asset_element has unnecessary union 
        // (for parents) here
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "   v1.name, v2.name AS type_name,"
            "   v.id_asset_element, v2.id_asset_device_type"
            " FROM"
            "   v_bios_asset_group_relation v,"
            "   t_bios_asset_element v1,"
            "   v_bios_asset_device v2"
            " WHERE "
            "   v.id_asset_group = :groupid AND"
            "   v1.id_asset_element = v.id_asset_element AND"
            "   v2.id_asset_element = v.id_asset_element"
        );
        // can return more than one row
        tntdb::Result result = st.set("groupid", element_id).
                                  select();
        log_debug("rows selected %u", result.size());
        
        for ( auto &row: result )
        {
            // device_name, required
            std::string device_name = "";
            row[0].get(device_name);
            assert ( !device_name.empty() );
            
            // device_type_name, required 
            std::string device_type_name = "";
            row[1].get(device_type_name);
            assert ( !device_type_name.empty() );

            // id_asset_element, required
            a_elmnt_id_t id_asset_element = 0;
            row[2].get(id_asset_element);
            assert ( id_asset_element );
            
            a_dvc_tp_id_t device_type_id = 0;
            row[2].get(device_type_id);
            assert ( device_type_id );

            log_debug ("for");
            log_debug ("device_name = %s", device_name.c_str());
            log_debug ("device_type_name = %s", device_name.c_str());
            log_debug ("asset_element_id = %" PRIu32, id_asset_element);
            log_debug ("device_type_id = %" PRIu16, device_type_id);
            
            resultdevices.insert (std::make_tuple(
                    id_asset_element, device_name, device_type_name,
                    device_type_id));
        } // end for
    }
    catch (const std::exception &e) {
        // internal error in database
        log_warning ("abort with err = '%s'", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
    }
    log_info("end select devices");
    zmsg_t* result = generate_return_power (resultdevices, resultpowers);
    log_info ("end normal");
    return result;
}

zmsg_t* get_return_power_topology_datacenter(const char* url, 
                                    asset_msg_t* getmsg)
{
    assert ( getmsg );
    assert ( asset_msg_id (getmsg) == ASSET_MSG_GET_POWER_DATACENTER );
    log_info ("start");
    a_elmnt_id_t   element_id = asset_msg_element_id  (getmsg);
    a_lnk_tp_id_t  linktype   = INPUT_POWER_CHAIN;

    log_info ("start select devices"); 
    // result set of found devices 
    std::set< device_info_t > resultdevices;
    try{
        tntdb::Connection conn = tntdb::connectCached(url);
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "   v.id_asset_element, v.name,"
            "   v.type_name, v.id_asset_device_type"
            " FROM"
            "   v_bios_asset_element_super_parent v"
            " WHERE :dcid IN (v.id_parent1, v.id_parent2 ,v.id_parent3,"
            "                   v.id_parent4)"
        );
        // can return more than one row
        log_debug ("element id %u", element_id);
        tntdb::Result result = st.set("dcid", element_id).select();
        log_debug("rows selected %u", result.size());
        
        for ( auto &row: result )
        {
            // id_asset_element, required
            a_elmnt_id_t id_asset_element = 0;
            row[0].get(id_asset_element);
            assert ( id_asset_element );
            
            // device_name, required
            std::string device_name = "";
            row[1].get(device_name);
            assert ( !device_name.empty() );
            
            // device_type_name, required 
            std::string device_type_name = "";
            row[2].get(device_type_name);
            assert ( !device_type_name.empty() );

            // device_type_id, required 
            a_dvc_tp_id_t device_type_id = 0;
            row[3].get(device_type_id);
            assert ( device_type_id );
            
            log_debug ("for");
            log_debug ("device_name = %s", device_name.c_str());
            log_debug ("device_type_name = %s", device_type_name.c_str());
            log_debug ("device_type_id = %" PRIu16, device_type_id);
            log_debug ("asset_element_id = %" PRIu32, id_asset_element);
            
            resultdevices.insert (std::make_tuple(
                    id_asset_element, device_name, 
                    device_type_name, device_type_id));
        } // end for
    }
    catch (const std::exception &e) {
        // internal error in database
        log_warning ("abort with err = '%s'", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
    }
    log_info("end select devices");

    log_info ("start select powers"); 
    //      all powerlinks are included into "resultpowers"
    std::set< powerlink_info_t > resultpowers;
    try{
        tntdb::Connection conn = tntdb::connectCached(url);
        // v_bios_asset_link are only devices, 
        // so there is no need to add more constrains
        tntdb::Statement st = conn.prepareCached(
            " SELECT"                
            "   v.src_out, v.id_asset_element_src, v.dest_in,"
            "   v.id_asset_element_dest"              
            " FROM"
            "   v_bios_asset_link v,"
            "   v_bios_asset_element_super_parent v1,"
            "   v_bios_asset_element_super_parent v2"
            " WHERE"
            "   v.id_asset_link_type = :linktypeid AND"
            "   v.id_asset_element_dest = v2.id_asset_element AND"
            "   ( :dcid IN (v2.id_parent1, v2.id_parent2 ,v2.id_parent3,"
            "               v2.id_parent4) ) AND"
            "   v.id_asset_element_src = v1.id_asset_element AND" 
            "   ( :dcid IN (v1.id_parent1, v1.id_parent2 ,v1.id_parent3,"
            "               v1.id_parent4) )"
        );
        // can return more than one row
        tntdb::Result result = st.set("dcid", element_id).
                                  set("linktypeid", linktype).
                                  select();
        log_debug("rows selected %u", result.size());
        
        // Go through the selected links
        for ( auto &row: result )
        {
            // src_out 
            std::string src_out = SRCOUT_DESTIN_IS_NULL;
            row[0].get(src_out);
            
            // id_asset_element_src, required
            a_elmnt_id_t id_asset_element_src = 0;
            row[1].get(id_asset_element_src);
            assert ( id_asset_element_src );
            
            // dest_in
            std::string dest_in = SRCOUT_DESTIN_IS_NULL;
            row[2].get(dest_in);
            
            // id_asset_element_dest, required
            a_elmnt_id_t id_asset_element_dest = 0;
            row[3].get(id_asset_element_dest);
            assert ( id_asset_element_dest );
            
            log_debug ("for");
            log_debug ("asset_element_id_src = %" PRIu32, 
                                                id_asset_element_src);
            log_debug ("asset_element_id_dest = %" PRIu32, 
                                                id_asset_element_dest);
            log_debug ("src_out = %s", src_out.c_str());
            log_debug ("dest_in = %s", dest_in.c_str());

            resultpowers.insert  (std::make_tuple(
                id_asset_element_src, src_out, 
                id_asset_element_dest, dest_in)); 
        } // end for
    }
    catch (const std::exception &e) {
        // internal error in database
        log_warning ("abort with err = '%s'", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
    }
    log_info ("end select powers");
    zmsg_t* result = generate_return_power (resultdevices, resultpowers);
    log_info ("end normal");
    return result;
}
