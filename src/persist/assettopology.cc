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
    \brief Functions for getting the topology (location andpower) from the
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
#include "assetmsg.h"
#include "common_msg.h"
#include "asset_types.h"

#include "assettopology.h"

// TODO HARDCODED CONSTANTS for asset device types

// TODO This parameter should be placed in configure file
// but now configure file doesn't exist. 
// So instead of it the constat would be used
#define MAX_RECURSION_DEPTH 6
#define INPUT_POWER_CHAIN 1

zmsg_t *process_assettopology (const char *database_url, asset_msg_t **message_p) {
    log_open ();
    log_set_level (LOG_DEBUG);

    assert (message_p);
    assert (database_url);
    zmsg_t *return_msg = NULL;
    if (*message_p) {
        asset_msg_t *message = *message_p;
        assert (message);

        int id = asset_msg_id (message);
        switch (id) {
            // TODO: Actually some of these messages don't make sense to be implemented (like ASSET_MSG_RETURN_LOCATION_TO),
            //       someone should sort them out and make a common zmsg errmsg for them. 
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
                log_info ("Processing of messages with ID = '%d' not implemented at the moment.\n", id);
                common_msg_t *common_msg = common_msg_new (COMMON_MSG_FAIL);
                assert (common_msg);
                common_msg_set_errmsg (common_msg,
                                       "Processing of messages with ID = '%d' not implemented at the moment.",
                                       id);
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
                return_msg = get_return_power_topology_from(database_url, message);
                assert (return_msg);
                break;
            }
            case ASSET_MSG_GET_POWER_TO:
            {
                return_msg = get_return_power_topology_to (database_url, message);
                assert (return_msg);
                break;
            }
            case ASSET_MSG_GET_POWER_GROUP:
            {
                return_msg = get_return_power_topology_group (database_url, message);
                assert (return_msg);
                break;
            }
            case ASSET_MSG_GET_POWER_DATACENTER:
            {
                return_msg = get_return_power_topology_datacenter (database_url, message);
                assert (return_msg);
                break;
            }
            default:
            {
                log_warning ("Unexpected message type received. Message ID: '%d'\n", id);
                
                common_msg_t *common_msg = common_msg_new (COMMON_MSG_FAIL);
                assert (common_msg);
                common_msg_set_errmsg (common_msg,
                                       "Unexpected message type received. Message ID: '%d'",
                                       id);
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
        log_error ("Pointer to null pointer passed as second argument 'asset_msg_t **message_p'.");
        return_msg = common_msg_encode_fail (0, 0,"Invalid asset message: Pointer to null pointer passed as second argument.", NULL);
        assert (return_msg);
        assert (is_common_msg (return_msg));
    }
    assert (return_msg); // safeguard non-NULL return value
    log_close ();
    return return_msg;
}

/**\brief A helper function, that transforms a Matroshka to Frame
 */
int matryoshka2frame (zmsg_t **matryoshka, zframe_t **frame )
{
    assert ( matryoshka );
    if ( *matryoshka ) {
        byte *buffer;
        size_t zmsg_size = zmsg_encode (*matryoshka, &buffer);

        // double check size
        // TODO after some time, remove the redundant check
        size_t check_size = 0;
        zframe_t *tmp_frame = zmsg_first (*matryoshka);
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

/**
 * \brief Helper function
 *
 * Calculates zframe_t size even for NULL value.
 *
 * \param frame - frame
 *
 * \return size of the frame
 */
size_t my_size(zframe_t* frame)
{
    if (frame == NULL)
        return 0;
    else
        return zframe_size (frame);
}

/**
 * \brief Selects group elements of specified type for the specified group.
 *  
 *  Paramenters "element_type_id" and "group_name" are used only in the 
 *  returned zmsg_t message.
 * 
 *  In case of success it generates the ASEET_MSG_RETURN_LOCATION_FROM. 
 *  In case of failure returns COMMON_MSG_FAIL.

 * \param url             - connection to database
 * \param element_id      - asset element id of group
 * \param element_type_id - asset_element_type_id of the group
 * \param group_name      - name of the group
 * \param filter_type     - id of the type of the searched elements
 *                          (from the table t_bios_asset_element_type)
 *
 * \return zmsg_t  - encoded ASEET_MSG_RETURN_LOCATION_FROM or 
 *                           COMMON_MSG_FAIL.
 */
zmsg_t* select_group_elements(
            const char* url             , uint32_t    element_id, 
            uint8_t     element_type_id , const char* group_name, 
            const char* dtype_name      , uint8_t     filtertype
        )
{
    assert ( element_id );  // id of the group should be specified
    assert ( element_type_id ); // type_id of the group
    assert ( ( filtertype >= asset_type::GROUP ) && ( filtertype <= 7 ) ); 
    // it can be only 1,2,3,4,5,6.7. 7 means - take all

    log_info ("start\n");
    log_debug ("element_id = %d\n", element_id);
    log_debug ("filter_type = %d\n", filtertype);
 
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
        tntdb::Result result = st.setInt("elementid", element_id).
                                  select();
        
        log_debug("rows selected %d\n", result.size());
        int i = 0;
        int rv = 0;

        zmsg_t* dcss     = zmsg_new();
        zmsg_t* roomss   = zmsg_new();
        zmsg_t* rowss    = zmsg_new();
        zmsg_t* rackss   = zmsg_new();
        zmsg_t* devicess = zmsg_new();

        for ( auto &row: result )
        {
            i++;
            uint32_t id = 0;
            row[0].get(id);
            assert ( id );
    
            std::string name = "";
            row[1].get(name);
            assert ( strcmp(name.c_str(), "") );

            uint16_t id_type = 0;
            row[2].get(id_type);
            assert ( id_type );
            
            std::string dtype_name = "";
            row[3].get(dtype_name);
            
            log_debug ("for\n");
            log_debug ("i = %d\n", i);
            log_debug ("id = %d\n", id);
            log_debug ("name = %s\n", name.c_str());
            log_debug ("id_type = %d\n", id_type);
            log_debug ("dtype_name = %s\n", dtype_name.c_str());

            // we are interested in this element if we are interested in 
            // all elements ( filtertype == 7) or if this element has 
            // exactly the type of the filter (filtertype == id_type)
            if ( ( filtertype == 7 ) || ( filtertype == asset_type::GROUP )
                    || ( filtertype == id_type ) )
            {
                // dcs, rooms, rows, racks, devices, groups are NULL
                // because we provide only first layer of inclusion
                zmsg_t* el = asset_msg_encode_return_location_from 
                              (id, id_type, name.c_str(), dtype_name.c_str(), 
                               NULL, NULL, NULL, NULL, NULL, NULL);
                assert ( el );
        
                // we are interested in this element
                log_debug ("created msg el for i = %d\n", i);

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
        
        zframe_t* dcs     = NULL;
        zframe_t* rooms   = NULL;
        zframe_t* rows    = NULL;
        zframe_t* racks   = NULL;
        zframe_t* devices = NULL;
        
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
        log_info ("end\n");
        return el;
    }
    catch (const std::exception &e) {
        log_warning ("abort with err = '%s'\n", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
    }
}

/**
 * \brief Select childs of specified type for the specified element 
 *  (element_id + element_type_id). 
 *  
 *  To select unlockated elements need to set element_id to 0.
 *  To select without the filter need to set a filtertype to 7.
 *
 * \param url             - connection to database
 * \param element_id      - id of the asset element
 * \param element_type_id - id of the type of the asset element
 * \param child_type_id   - type id of the child asset elements
 * \param is_recursive    - if the search recursive or not
 * \param current_depth   - a recursion parameter, started from 1
 * \param filter_type     - id of the type of the searched elements
 *
 * \return zframe_t - list of the childs of secified type according 
 *                    to the filter filter_type. (it is a Matryoshka).
 */
zframe_t* select_childs(
    const char* url             , uint32_t element_id, 
    uint8_t     element_type_id , uint8_t child_type_id, 
    bool        is_recursive    , uint32_t current_depth, 
    uint8_t     filtertype)
{
    assert ( child_type_id );   // is required
    assert ( ( filtertype >= asset_type::GROUP ) && ( filtertype <= 7 ) ); 
    // it can be only 1,2,3,4,5,6.7. 7 means - take all

    log_info ("start select_childs\n");
    log_debug ("depth = %d\n", current_depth);
    log_debug ("element_id = %d\n", element_id);
    log_debug ("element_type_id = %d\n", element_type_id);
    log_debug ("child_type_id = %d\n", child_type_id);

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
            result = st.setInt("elementid", element_id).
                        setInt("elementtypeid", element_type_id).
                        setInt("childtypeid", child_type_id).
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
            result = st.setInt("childtypeid", child_type_id).
                        select();
        }
        log_debug("rows selected %d\n", result.size());
        int rv = 0;
        zmsg_t* ret = zmsg_new();
        assert (ret);
        int i = 0;
        for ( auto &row: result )
        {
            i++;
            uint32_t id = 0;
            row[0].get(id);
            assert ( id );
    
            std::string name = "";
            row[1].get(name);
            assert ( strcmp(name.c_str(), "") );

            uint16_t id_type = 0;
            row[2].get(id_type);
            assert ( id_type );
            
            std::string dtype_name = "";
            row[3].get(dtype_name);
            
            log_debug ("for\n");
            log_debug ("i = %d\n", i);
            log_debug ("id = %d\n", id);
            log_debug ("name = %s\n", name.c_str());
            log_debug ("id_type = %d\n", id_type);
            log_debug ("dtype_name = %s\n", dtype_name.c_str());

            zframe_t* dcs     = NULL;
            zframe_t* rooms   = NULL;
            zframe_t* rows    = NULL;
            zframe_t* racks   = NULL;
            zframe_t* devices = NULL;
            zmsg_t*   grp     = NULL;
            
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
                    log_info ("start select_rooms\n");
                    rooms = select_childs (url, id, child_type_id, 
                                asset_type::ROOM, is_recursive, 
                                current_depth + 1, filtertype);
                    log_info ("end select_rooms\n");
                }

                // Select rows only for datacenters and rooms
                // and TODO filter
                if ( (  ( child_type_id == asset_type::DATACENTER ) ||
                        ( child_type_id == asset_type::ROOM ) )     && 
                     ( 4 <= filtertype ) )
                {
                    log_info ("start select_rows\n");
                    rows  = select_childs (url, id, child_type_id,
                                asset_type::ROW, is_recursive, 
                                current_depth + 1, filtertype);
                    log_info ("end select_rows\n");
                }
                
                // Select racks only for datacenters, rooms, rows
                // and TODO filter
                if ( (  ( child_type_id == asset_type::DATACENTER)  ||
                        ( child_type_id == asset_type::ROOM )       ||
                        ( child_type_id == asset_type::ROW ) )     && 
                     ( 5 <= filtertype ) )
                {
                    log_info ("start select_racks\n");
                    racks   = select_childs (url, id, child_type_id, 
                                asset_type::RACK, is_recursive, 
                                current_depth + 1, filtertype);
                    log_info ("end select_racks\n");
                }
                
                // Select devices only for datacenters, rooms, rows, racks
                // and TODO filter
                if ( (  ( child_type_id == asset_type::DATACENTER)  ||
                        ( child_type_id == asset_type::ROOM )       ||
                        ( child_type_id == asset_type::ROW )        ||
                        ( child_type_id == asset_type::RACK ) )     &&
                     ( 6 <= filtertype ) )
                {
                    log_info ("start select_devices\n");
                    devices = select_childs (url, id, child_type_id,
                                asset_type::DEVICE, is_recursive, 
                                current_depth + 1, filtertype);
                    log_info ("end select_devices\n");
                }
                
                // TODO filter
                // if it is a group, then do a special processing
                if (    ( child_type_id == asset_type::GROUP) && 
                        (   ( asset_type::GROUP == filtertype ) || 
                            ( filtertype == 7 ) 
                        ) )
                {
                    log_info ("start select elements of the grp\n");
                    grp = select_group_elements (url, id, asset_type::GROUP, 
                                name.c_str(), dtype_name.c_str(), filtertype);
                    log_info ("end select elements of the grp\n");
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
                
                zmsg_t* el;
                if (    ( child_type_id == asset_type::GROUP ) && 
                        ( is_recursive ) )
                    el = grp;   // because of the special group processing
                else 
                    el = asset_msg_encode_return_location_from 
                                (id, id_type, name.c_str(), 
                                 dtype_name.c_str(), dcs, rooms, 
                                 rows, racks, devices, NULL);
                assert ( el );
                log_debug ("created msg el for i = %d\n",i);
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
        log_info ("end\n");
        return res;
    }
    catch (const std::exception &e) {
        log_warning ("abort with err = '%s'\n", e.what());
        return NULL;
    }
}

/**
 * \brief This function processes the ASSET_MSG_GET_LOCATION_FROM message
 *
 * To correct processing all fields of the message should be set up 
 * according specification.
 * 
 * To select unlockated elements need to set element_id to 0.
 * For unlockated elements only a non recursive search is provided.
 * To select without the filter need to set a filtertype to 7.
 *
 * In case of success it generates the ASSET_MSG_RETURN_LOCATION_FROM. 
 * In case of failure returns COMMON_MSG_FAIL.
 * 
 * It doesn't destroy the getmsg.
 *
 * \param url - the connection to database.
 * \param msg - the message of the type ASSET_MSG_GET_LOCATION_FROM 
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       ASSET_MSG_RETURN_LOCATION_FROM
 */
zmsg_t* get_return_topology_from(const char* url, asset_msg_t* getmsg)
{
    assert ( getmsg );
    assert (url);
    assert ( asset_msg_id (getmsg) == ASSET_MSG_GET_LOCATION_FROM );
    log_info ("start\n");

    zmsg_t *return_msg = NULL;

    uint32_t element_id   = asset_msg_element_id  (getmsg);
    uint8_t filter_type = asset_msg_filter_type (getmsg);
    int is_recursive = 0;
    int type_id = 0;

    tntdb::Connection conn;
    try {
        conn = tntdb::connectCached (url);
    }
    catch (const std::exception& e) {
        log_warning ("Error when connecting to database '%s': %s", url, e.what());
        common_msg_t *common_msg = common_msg_new (COMMON_MSG_FAIL);
        assert (common_msg);
        common_msg_set_errmsg (common_msg,
                               "Error when connecting to database '%s': %s",
                                url, e.what());
        zmsg_t *return_msg = common_msg_encode (&common_msg);
        assert (return_msg);
        assert (common_msg == NULL);
        assert (is_common_msg (return_msg));
        return return_msg;
    }

    // element_id == 0 => we are looking for unlocated elements; recursive := false; 
    if (element_id != 0) {
        is_recursive = asset_msg_recursive (getmsg);
        // get type
        try {
            std::string tmp = "SELECT id_type FROM t_bios_asset_element WHERE id_asset_element = ";
            tmp.append (std::to_string (element_id));
            tntdb::Statement st = conn.prepare (tmp.c_str());        
            tntdb::Value val = st.selectValue();
            val.get (type_id);           
        }
        catch (const std::exception& e) {
            log_warning ("Error when executing statement: %s", e.what());
            common_msg_t *common_msg = common_msg_new (COMMON_MSG_FAIL);
            assert (common_msg);
            common_msg_set_errmsg (common_msg,
                                   "Error executing statement: %s",
                                   e.what());
            zmsg_t *return_msg = common_msg_encode (&common_msg);
            assert (return_msg);
            assert (common_msg == NULL);
            assert (is_common_msg (return_msg));
            return return_msg;
        }
    } 
    
    zframe_t* dcs     = NULL;
    zframe_t* rooms   = NULL;
    zframe_t* rows    = NULL;
    zframe_t* racks   = NULL;
    zframe_t* devices = NULL;
    zframe_t* grps    = NULL;
    
    std::string name = "";
    std::string dtype_name = "";

    // select additional information about starting device
    if ( element_id != 0 )
    {
        // if looking for a lockated elements
        try{
            tntdb::Statement st = conn.prepareCached(
                " SELECT"
                "    v.name, v1.name as dtype_name"                  
                " FROM v_bios_asset_element v"
                "    LEFT JOIN v_bios_asset_device v1"
                "      ON (v.id = v1.id_asset_element)"
                " WHERE v.id = :id"
            );
    
            tntdb::Row row = st.setInt("id", element_id).
                                selectRow();
            
            row[0].get(name);
            assert ( strcmp(name.c_str(), "") );

            row[1].get(dtype_name);
        }
        catch (const tntdb::NotFound &e) {
            // element with specified id was not found
            log_warning ("abort select element with err = '%s'\n", e.what());
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_NOTFOUND, 
                                                        e.what(), NULL);
        }
        catch (const std::exception &e) {
            // internal error in database
            log_warning ("abort select element with err = '%s'\n", e.what());
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
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
        log_info ("start select_rooms\n");
        rooms = select_childs (url, element_id, type_id, asset_type::ROOM,
                        is_recursive, 1, filter_type);
        if ( rooms == NULL )
        {
            zframe_destroy (&dcs);
            log_warning ("end abnormal\n");
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                    "rooms error", NULL);
        }
        log_info ("end select_rooms\n");
    }
    // Select rows
    // only for rooms, datacenters, unlockated
    // TODO filter
    if ( ( ( type_id == asset_type::DATACENTER)  ||
           ( type_id == asset_type::ROOM )       || 
           ( element_id == 0 ) ) &&
         ( 4 <= filter_type ) )
    {
        log_info ("start select_rows\n");
        rows = select_childs (url, element_id, type_id, asset_type::ROW,
                        is_recursive, 1, filter_type);
        if ( rows == NULL )
        {
            zframe_destroy (&dcs);
            zframe_destroy (&rooms);
            log_warning("end abnormal\n");
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                    "rows error", NULL);
        }
        log_info ("end select_rows\n");
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
        log_info ("start select_racks\n");
        racks = select_childs (url, element_id, type_id, asset_type::RACK,
                        is_recursive, 1, filter_type);
        if ( racks == NULL )
        {
            zframe_destroy (&dcs);
            zframe_destroy (&rooms);
            zframe_destroy (&rows);
            log_warning ("end abnormal\n");
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                    "racks error", NULL);
        }
        log_info ("end select_racks\n");
    }
    // Select devices
    // only for rooms, datacenters, rows, racks, unlockated
    // TODO filter
    if ( ( ( type_id == asset_type::DATACENTER)  ||
           ( type_id == asset_type::ROOM )       ||
           ( type_id == asset_type::ROW )        ||
           ( type_id == asset_type::DEVICE )     || 
           ( element_id == 0 )  ) &&
         ( 6 <= filter_type ) )
    {
        log_info ("start select_devices\n");
        devices = select_childs (url, element_id, type_id, asset_type::DEVICE,
                        is_recursive, 1, filter_type);
        if ( devices == NULL )
        {
            zframe_destroy (&dcs);
            zframe_destroy (&rooms);
            zframe_destroy (&rows);
            zframe_destroy (&racks);
            log_warning ("end abnormal\n");
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                    "devices error", NULL);
        }
        log_info ("end select_devices\n");
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
        log_info ("start select_grps\n");
        grps = select_childs (url, element_id, type_id, asset_type::GROUP,
                        is_recursive, 1, filter_type);
        if ( grps == NULL )
        {
            zframe_destroy (&dcs);
            zframe_destroy (&rooms);
            zframe_destroy (&rows);
            zframe_destroy (&racks);
            zframe_destroy (&devices);
            log_warning ("end abnormal\n");
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                    "groups error", NULL);
        }
        log_info ("end select_grps\n");
    }
    log_info ("creating return element\n");
    zmsg_t* el = asset_msg_encode_return_location_from 
                       (element_id, type_id, name.c_str(), 
                        dtype_name.c_str(), dcs, rooms, rows, 
                        racks, devices, grps);
    log_info ("end normal\n");
    return el;
}

/* \brief Helper function for testing 
 *  
 * Compares the element in the ASSET_MSG_RETURN_LOCATION_FROM or in 
 * the ASSET_MSG_RETURN_LOCATION_TO field by field with specified values.
 *
 * \param rmsg       - message with element
 * \param id         - id (asset_elememt_id) of the element compared to
 * \param id_type    - id of the type of the element compared to
 * \param name       - name of the element compared to
 * \param dtype_name - name of the precise element type of the element 
 *                      compared to (available only for groups and devices).
 *
 * \return true  - if the element is the same
 *         false - if elements are different
 */
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

/**
 * \brief Helper function for testing
 * 
 * Converts a frame (from ASSET_MSG_LOCATION_FROM message) into 
 * the std::set of edges in the tree (child,parent);
 *
 * \param frame      - frame to convert
 * \param parent_id  - id (asset_elememt_id) of the root
 * \param id_type    - id of the type of the root
 * \param name       - name of the root compared to
 * \param dtype_name - name of the precise element type of root 
 *                      (available only for groups and devices).
 * 
 * \return std::set of edges
 */
edge_lf print_frame_to_edges (zframe_t* frame, uint32_t parent_id, 
                uint8_t type, std::string name, std::string dtype_name)
{    
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    edge_lf result, result1;

    zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );
     
    zmsg_t* pop = NULL;
    while ( ( pop = zmsg_popmsg (zmsg) ) != NULL )
    { 
        asset_msg_t* item = asset_msg_decode (&pop); // zmsg_t is freed
        assert ( item );
//        asset_msg_print (item);
        
        result.insert(std::make_tuple (
                    asset_msg_element_id(item), 
                    asset_msg_type(item), 
                    asset_msg_name(item), 
                    asset_msg_type_name(item), 
                    parent_id, type, name, dtype_name) ); 
        log_debug ("parent_id = %d\n", parent_id );
        
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

/**
 * \brief Helper function for testing
 * 
 * prints a frame (from ASSET_MSG_LOCATION_FROM message)
 *
 * \param frame      - frame to print
 * \param parent_id  - id (asset_elememt_id) of the parent
 * 
 */
void print_frame (zframe_t* frame, uint32_t parent_id)
{    
    byte* buffer = zframe_data (frame);
    assert ( buffer );

    zmsg_t* zmsg = zmsg_decode (buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );
     
    zmsg_t* pop = NULL;
    while ( ( pop = zmsg_popmsg (zmsg) ) != NULL )
    { 
        asset_msg_t* item = asset_msg_decode (&pop); // zmsg_t is freed
        assert ( item );
        asset_msg_print (item);
        log_debug("parent_id = %d\n", parent_id );
        
        zframe_t* fr = asset_msg_dcs (item);
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

/**
 * \brief Recursivly selects the parents of the element until the top 
 *  unlocated element.
 *
 * Generates the ASSET_MSG_RETURN_TOPOLOGY_TO message, but in inverse 
 * order (the specified element would be on the top level, but the top 
 * location would be at the bottom level);
 *
 * \param url             - the connection to database.
 * \param element_id      - the element id
 * \param element_type_id - id of the element's type
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or 
 *                      ASSET_MSG_RETURN_TOPOLOGY_TO
 */
zmsg_t* select_parents (const char* url, uint32_t element_id, 
                        uint8_t element_type_id)
{
    assert ( element_id );      // is required
    assert ( element_type_id ); // is required

    log_info ("start\n");
    log_debug ("element_id = %d\n", element_id);
    log_debug ("element_type_id = %d\n", element_type_id);

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
        tntdb::Row row = st.setInt("elementid", element_id).
                            setInt("elementtypeid", element_type_id).
                            selectRow();
    
        uint32_t parent_id = 0;
        uint16_t parent_type_id = 0;

        std::string dtype_name = ""; 
        std::string name = "";
        row[0].get(parent_id);
        row[1].get(parent_type_id);
        row[2].get(name);
        row[3].get(dtype_name);

        log_debug("rows selected %d, parent_id = %d, parent_type_id = %d\n",
                            1,  parent_id, parent_type_id);
        
        if ( parent_id != 0 )
        {  
            zmsg_t* parent = select_parents (url, parent_id, parent_type_id);
            if ( is_asset_msg (parent) )
                return asset_msg_encode_return_location_to (element_id, 
                            element_type_id, name.c_str(), 
                            dtype_name.c_str(), parent);
            else if ( is_common_msg (parent) )
                return parent;
            else
                return common_msg_encode_fail (BIOS_ERROR_DB, 
                        DB_ERROR_INTERNAL, "UNSUPPORTED RETURN MESSAGE TYPE", 
                        NULL);
        }
        else
        {
            log_info ("but this element has no parent\n");
            return asset_msg_encode_return_location_to (element_id, 
                    element_type_id, name.c_str(), dtype_name.c_str(), 
                    zmsg_new());
        }
    }
    catch (const tntdb::NotFound &e) {
        // element with specified type was not found
        log_warning ("abort with err = '%s'\n", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_NOTFOUND, 
                                                        e.what(), NULL);
    }
    catch (const std::exception &e) {
        // internal error in database
        log_warning ("abort with err = '%s'\n", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
    }
}

/**
 * \brief This function processes the ASSET_MSG_GET_LOCATION_TO message
 *
 * In case of success it generates the ASSET_MSG_RETURN_LOCATION_TO. 
 * In case of failure returns COMMON_MSG_FAIL.
 * 
 * \param url - the connection to database.
 * \param msg - the message of the type ASSET_MSG_GET_LOCATION_TO 
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       ASSET_MSG_RETURN_LOCATION_TO
 */
zmsg_t* get_return_topology_to(const char* url, asset_msg_t* getmsg)
{
    assert ( getmsg );
    assert (url);
    assert ( asset_msg_id (getmsg) == ASSET_MSG_GET_LOCATION_TO );
    log_info ("start\n");
    uint32_t element_id = asset_msg_element_id (getmsg);
    int type_id = 0;    

    try {
        tntdb::Connection conn = tntdb::connectCached (url);
        /* This didn't work
        tntdb::Statement st = conn.prepare (
            "SELECT id_type FROM t_bios_asset_element"
            "WHERE id_asset_element = :v1");        
        tntdb::Value val = st.setInt("v1", element_id).selectValue();
        */
        std::string tmp = "SELECT id_type FROM t_bios_asset_element WHERE id_asset_element = ";
        tmp.append (std::to_string (element_id));
        tntdb::Statement st = conn.prepare (tmp.c_str());        
        tntdb::Value val = st.selectValue();
        val.get (type_id);
    }
    catch (const std::exception& e) {
        log_warning ("Error connecting to database or querying a database '%s': %s\n", url, e.what());
        common_msg_t *common_msg = common_msg_new (COMMON_MSG_FAIL);
        assert (common_msg);
        common_msg_set_errmsg (common_msg,
                               "Error connecting to database or querying a database '%s': %s",
                               url, e.what());
        zmsg_t *return_msg = common_msg_encode (&common_msg);
        assert (return_msg);
        assert (common_msg == NULL);
        assert (is_common_msg (return_msg));
        return return_msg;
    }

    zmsg_t* result = select_parents (url, element_id, type_id);

    if (is_asset_msg (result)) {
        zmsg_t *dup = zmsg_dup (result);
        asset_msg_t *am = asset_msg_decode (&dup);
        asset_msg_t *orig = am;
        assert (asset_msg_id (am) == ASSET_MSG_RETURN_LOCATION_TO );
        bool go = false;
        do {
            log_info ("element_id = %d\n", (int) asset_msg_element_id (am));
            log_info ("type = %d\n", (int) asset_msg_type (am));
            if (zmsg_size (asset_msg_msg (am)) != 0) {
                log_info ("inner msg is not null\n");
                zmsg_t *inner = asset_msg_get_msg (am);
                assert (inner);
                am = asset_msg_decode (&inner);
                assert (am);
                go = true;
            } else {
                go  = false;
            }
        } while (go == true);

    }


    log_info ("end\n");
    return result;
}

/**
 * \brief This function processes the ASSET_MSG_GET_POWER_FROM message
 *
 * In case of success it generates the ASSET_MSG_RETURN_POWER. 
 * In case of failure returns COMMON_MSG_FAIL.
 * 
 * A single powerchain link is coded as "A:B:C:D" string 
 * ("src_socket:src_id:dst_socket:dst_id").
 * If A or C is zero than A or C it was not srecified in database 
 * (it was NULL). 
 * 
 * \param url - the connection to database.
 * \param msg - the message of the type ASSET_MSG_GET_POWER_FROM 
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       ASSET_MSG_RETURN_POWER
 */ 
zmsg_t* get_return_power_topology_from(const char* url, asset_msg_t* getmsg)
{
    assert ( getmsg );
    assert ( asset_msg_id (getmsg) == ASSET_MSG_GET_POWER_FROM );
    log_info ("start\n");
    uint32_t element_id   = asset_msg_element_id  (getmsg);
    uint8_t  linktype = INPUT_POWER_CHAIN;

    std::string device_name = "";
    std::string device_type_name = "";

    // select information about the start device
    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "   v.name, v1.name as type_name"
            " FROM"
            "   v_bios_asset_element v"
            " LEFT JOIN  v_bios_asset_device v1"
            "   ON ( v1.id_asset_element = v.id )"
            " WHERE v.id = :id"
        );
        tntdb::Row row = st.setInt("id", element_id).
                            selectRow();
        
        // device name, required
        row[0].get(device_name);
        assert ( device_name != "" ); // database is corrupted
        
        // device type name, would be NULL if it is not a device
        row[1].get(device_type_name);
    }
    catch (const tntdb::NotFound &e) {
        // device with specified id was not found
        log_warning ("abort with err = '%s'\n", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_NOTFOUND, 
                                                        e.what(), NULL);
    }
    catch (const std::exception &e) {
        // internal error in database
        log_warning ("abort with err = '%s'\n", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
    }
    
    // check, if selected element has a device type
    if ( device_type_name == "" )
    {   // than it is not a device
        log_warning ("abort with err = '%s %d %s'\n", 
                        "specified element id =", element_id, 
                        " is not a device");
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_BADINPUT, 
                                        "specified element is not a device", 
                                        NULL);
    }
    zlist_t* powers = zlist_new();
    assert ( powers );
    zlist_set_duplicator (powers, void_dup);

    zframe_t* devices = NULL;
 
    std::set<std::tuple<int,std::string,std::string>> resultdevices;
    // ( id,  device_name, device_type_name )
    resultdevices.insert (std::make_tuple(
                        element_id, device_name, device_type_name));
    
    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "  v.id_asset_element_dest, v.src_out, v.dest_in, v.dest_name,"
            "  v.dest_type_name"
            " FROM"
            "  v_bios_asset_link_topology v"
            " WHERE"
            "  v.id_asset_link_type = :idlinktype AND"
            "  v.id_asset_element_src = :id"
        );
        // can return more than one value
        tntdb::Result result = st.setInt("id", element_id).
                                  setInt("idlinktype", linktype).
                                  select();
        
        char buff[28];     // 10+3+3+10+ safe 2
        
        // Go through the selected links
        for ( auto row: result )
        {
            // id_asset_element_dest, requiured
            uint32_t id_asset_element_dest = 0;
            row[0].get(id_asset_element_dest);
            assert ( id_asset_element_dest );
   
            // src_out 
            uint32_t src_out = 0;
            row[1].get(src_out);
            
            // dest_in
            uint32_t dest_in = 0;
            row[2].get(dest_in);
            
            // device_name, required
            std::string device_name = "";
            row[3].get(device_name);
            assert ( device_name != "" );

            // device_type_name, requiured
            std::string device_type_name = "";
            row[4].get(device_type_name);
            assert ( device_type_name != "" );

            log_debug ("for\n");
            log_debug ("asset_element_id_src = %d\n", element_id);
            log_debug ("asset_element_id_dest = %d\n", id_asset_element_dest);
            log_debug ("src_out = %d\n", src_out);
            log_debug ("dest_in = %d\n", dest_in);
            log_debug ("device_name = %s\n", device_name.c_str());
            log_debug ("device_type_name = %s\n", device_type_name.c_str());

            sprintf(buff, "%d:%d:%d:%d", src_out, element_id, dest_in, 
                            id_asset_element_dest);
            zlist_push(powers, buff);
            
            resultdevices.insert (std::make_tuple(
                    id_asset_element_dest, device_name, device_type_name));
        } // end for
    }
    catch (const std::exception &e) {
        // internal error in database
        zlist_destroy (&powers);
        zframe_destroy (&devices);
        log_warning ("abort with err = '%s'\n", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
    }
    
    zmsg_t* ret = zmsg_new();
    assert ( ret );
    for ( auto it = resultdevices.begin(); it != resultdevices.end(); ++it )
    {
        auto adevice = *it;

    // tuple: ( id,  device_name, device_type_name )
    // encode: id, device_type_name, device_name
        zmsg_t* el = asset_msg_encode_powerchain_device
                    (std::get<0>(adevice), (std::get<2>(adevice)).c_str(), 
                    (std::get<1>(adevice)).c_str() );
        int rv = zmsg_addmsg ( ret, &el);
        assert ( rv != -1 );
        assert ( el == NULL );
    }

    int rv = matryoshka2frame (&ret, &devices);
    assert ( rv == 0 );
    zmsg_t* result = asset_msg_encode_return_power (devices, powers);
    zlist_destroy (&powers);
    zframe_destroy (&devices);
    log_info ("end normal\n");
    return result;
}

/**
 * \brief Helper function for testing
 * 
 * Prints a frame of devices (from ASSET_MSG_POWER_FROM message)
 *
 * \param frame - frame to print
 */
void print_frame_devices (zframe_t* frame)
{    
    byte* buffer = zframe_data (frame);
    assert ( buffer );

    zmsg_t* zmsg = zmsg_decode (buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );
     
    zmsg_t* pop = NULL;
    while ( ( pop = zmsg_popmsg (zmsg) ) != NULL )
    { 
        asset_msg_t* item = asset_msg_decode (&pop);
        assert ( item );
        asset_msg_print (item);
        asset_msg_destroy (&item);
        assert ( pop == NULL );
    }
   zmsg_destroy (&zmsg);
}

/**
 * \brief This function processes the ASSET_MSG_GET_POWER_TO message
 *
 * In case of success it generates the ASSET_MSG_RETURN_POWER. 
 * In case of failure returns COMMON_MSG_FAIL.
 * 
 * A single powerchain link is coded as "A:B:C:D" string 
 * ("src_socket:src_id:dst_socket:dst_id").
 * If A or C is zero than A or C it was not srecified in database 
 * (it was NULL). 
 *
 * \param url - the connection to database.
 * \param msg - the message of the type ASSET_MSG_GET_POWER_TO
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       ASSET_MSG_RETURN_POWER
 */ 
zmsg_t* get_return_power_topology_to (const char* url, asset_msg_t* getmsg)
{
    assert ( getmsg );
    assert ( asset_msg_id (getmsg) == ASSET_MSG_GET_POWER_TO );
    log_info ("start\n");
    uint32_t element_id   = asset_msg_element_id  (getmsg);
    uint8_t  linktype = INPUT_POWER_CHAIN;

    std::string device_name = "";
    std::string device_type_name = "";
    
    // select information about the start device
    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "   v.name, v1.name as type_name"
            " FROM"
            "   v_bios_asset_element v"
            " LEFT JOIN  v_bios_asset_device v1"
            "   ON ( v1.id_asset_element = v.id )"
            " WHERE v.id = :id"
        );
        tntdb::Row row = st.setInt("id", element_id).
                            selectRow();
        
        // device name, required
        row[0].get(device_name);
        assert ( device_name != "" ); // database is corrupted
        
        // device type name, would be NULL if it is not a device
        row[1].get(device_type_name);
    }
    catch (const tntdb::NotFound &e) {
        // device with specified id was not found
        log_warning ("abort with err = '%s'\n", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_NOTFOUND, 
                                                        e.what(), NULL);
    }
    catch (const std::exception &e) {
        // internal error in database
        log_warning ("abort with err = '%s'\n", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
    }
    
    // check, if selected element has a device type
    if ( device_type_name == "" )
    {   // than it is not a device
        log_warning ("abort with err = '%s %d %s'\n", 
                "specified element id =", element_id, " is not a device");
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_BADINPUT, 
                            "specified element is not a device", NULL);
    }

    std::set<std::tuple<int,std::string,std::string>> newdevices, 
                                                      resultdevices;
                                                      
    // ( id,  device_name, device_type_name )
    auto adevice = std::make_tuple(element_id, device_name, device_type_name);
    resultdevices.insert (adevice);
    newdevices.insert (adevice);

    zlist_t* powers = zlist_new();
    assert ( powers );
    zlist_set_duplicator (powers, void_dup);

    bool ncontinue = true;
    while ( ncontinue )
    {
        uint32_t cur_element_id = std::get<0>(adevice);
        
        try{
            tntdb::Connection conn = tntdb::connectCached(url);

            tntdb::Statement st = conn.prepareCached(
                " SELECT"
                "  v.id_asset_element_src, v.src_out, v.dest_in, v.src_name,"
                "  v.src_type_name"
                " FROM"
                "  v_bios_asset_link_topology v"
                " WHERE"
                "  v.id_asset_link_type = :idlinktype AND"
                "  v.id_asset_element_dest = :id"
            );
            
            // can return more than one value
            tntdb::Result result = st.setInt("id", cur_element_id).
                                      setInt("idlinktype", linktype).
                                      select();
            
            char buff[28];     // 10+3+3+10+ safe 2
            
            // Go through the selected links
            for ( auto &row: result )
            {
                // id_asset_element_dest, requiured
                uint32_t id_asset_element_src = 0;
                row[0].get(id_asset_element_src);
                assert ( id_asset_element_src );
       
                // src_out 
                uint32_t src_out = 0;
                row[1].get(src_out);
                
                // dest_in
                uint32_t dest_in = 0;
                row[2].get(dest_in);
                
                // device_name_src, required
                std::string device_name_src = "";
                row[3].get(device_name_src);
                assert ( device_name_src != "" );
    
                // device_type_name_src, requiured
                std::string device_type_name_src = "";
                row[4].get(device_type_name_src);
                assert ( device_type_name_src != "" );
    
                log_debug ("for\n");
                log_debug ("asset_element_id_dest = %d\n", cur_element_id);
                log_debug ("asset_element_id_src = %d\n", 
                                                    id_asset_element_src);
                log_debug ("src_out = %d\n", src_out);
                log_debug ("dest_in = %d\n", dest_in);
                log_debug ("device_name_src = %s\n", device_name_src.c_str());
                log_debug ("device_type_name_src = %s\n", 
                                                device_type_name_src.c_str());
    
                sprintf(buff, "%d:%d:%d:%d", src_out, id_asset_element_src, 
                                                    dest_in, cur_element_id);
                zlist_push(powers, buff);

                newdevices.insert (std::make_tuple(
                        id_asset_element_src, device_name_src, 
                        device_type_name_src));
            } // end for
        }
        catch (const std::exception &e) {
            // internal error in database
            zlist_destroy (&powers);
            log_warning ("abort with err = '%s'\n", e.what());
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
        }

        // find the next dest device 
        ncontinue = false;
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
    }
    zmsg_t* ret = zmsg_new();
    assert ( ret );
    for ( auto it = resultdevices.begin(); it != resultdevices.end(); ++it )
    {
        adevice = *it;

        zmsg_t* el = asset_msg_encode_powerchain_device
                    (std::get<0>(adevice), (std::get<2>(adevice)).c_str(), 
                    (std::get<1>(adevice)).c_str() );
        int rv = zmsg_addmsg (ret, &el);
        assert ( rv != -1 );
        assert ( el == NULL );
    }
    
    zframe_t* devices = NULL;
    int rv = matryoshka2frame (&ret, &devices);
    assert ( rv == 0 );
    
    zmsg_t* result = asset_msg_encode_return_power (devices, powers);
    zlist_destroy (&powers);
    zframe_destroy (&devices);
    log_info ("end normal\n");
    return result;
}

/**
 * \brief This function processes the ASSET_MSG_GET_POWER_GROUP message
 *
 * In case of success it generates the ASSET_MSG_RETURN_POWER. 
 * In case of failure returns COMMON_MSG_FAIL.
 * 
 * Returns all devices in the group and returns all power links between them.
 * Links that goes outside the group are not returned.
 *
 * A single powerchain link is coded as "A:B:C:D" string 
 * ("src_socket:src_id:dst_socket:dst_id").
 * If A or C is zero then A or C were not srecified in database (were NULL). 
 *
 * \param url - the connection to database.
 * \param msg - the message of the type ASSET_MSG_GET_POWER_GROUP 
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       ASSET_MSG_RETURN_POWER
 */ 
zmsg_t* get_return_power_topology_group(const char* url, asset_msg_t* getmsg)
{
    assert ( getmsg );
    assert ( asset_msg_id (getmsg) == ASSET_MSG_GET_POWER_GROUP );
    log_info ("start\n");
    uint32_t element_id   = asset_msg_element_id  (getmsg);
    uint8_t  linktype = INPUT_POWER_CHAIN;

    // powers
    zlist_t* powers = zlist_new();
    assert ( powers );
    zlist_set_duplicator (powers, void_dup);
    
    log_info("start select powers\n"); 
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
        tntdb::Result result = st.setInt("groupid", element_id).
                                  setInt("linktypeid", linktype).
                                  select();
        
        char buff[28];     // 10+3+3+10+ safe 2
        
        // Go through the selected links
        for ( auto &row: result )
        {
            // src_out 
            uint32_t src_out = 0;
            row[0].get(src_out);
            
            // id_asset_element_src, required
            uint32_t id_asset_element_src = 0;
            row[1].get(id_asset_element_src);
            assert ( id_asset_element_src );
            
            // dest_in
            uint32_t dest_in = 0;
            row[2].get(dest_in);
            
            // id_asset_element_dest, required
            uint32_t id_asset_element_dest = 0;
            row[3].get(id_asset_element_dest);
            assert ( id_asset_element_dest );
            
            log_debug ("for\n");
            log_debug ("asset_element_id_src = %d\n", id_asset_element_src);
            log_debug ("asset_element_id_dest = %d\n", id_asset_element_dest);
            log_debug ("src_out = %d\n", src_out);
            log_debug ("dest_in = %d\n", dest_in);

            sprintf(buff, "%d:%d:%d:%d", src_out, id_asset_element_src, 
                                            dest_in, id_asset_element_dest);
            zlist_push (powers, buff);
        } // end for
    }
    catch (const std::exception &e) {
        // internal error in database
        zlist_destroy (&powers);
        log_warning ("abort with err = '%s'\n", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
    }
    // powers is ok
    log_info("end select powers\n");
    log_info("start select the devices\n"); 
    // devices
    zmsg_t* ret = zmsg_new();
    assert ( ret );
    try{
        tntdb::Connection conn = tntdb::connectCached(url);
        // select is done from pure t_bios_asset_element, 
        // because v_bios_asset_element has unnecessary union 
        // (for parents) here
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "   v1.name, v2.name AS type_name, v.id_asset_element"
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
        tntdb::Result result = st.setInt("groupid", element_id).
                                  select();
        
        log_debug("rows selected %d\n", result.size()); 
        for ( auto &row: result )
        {
            // device_name, required
            std::string device_name = "";
            row[0].get(device_name);
            assert ( device_name != "" );
            
            // device_type_name, required 
            std::string device_type_name = "";
            row[1].get(device_type_name);
            assert ( device_type_name != "" );

            // id_asset_element, required
            uint32_t id_asset_element = 0;
            row[2].get(id_asset_element);
            assert ( id_asset_element );

            log_debug ("for\n");
            log_debug ("device_name = %s\n", device_name.c_str());
            log_debug ("device_type_name = %s\n", device_name.c_str());
            log_debug ("asset_element_id = %d\n", id_asset_element);
            zmsg_t* el = asset_msg_encode_powerchain_device
                            (id_asset_element, device_type_name.c_str(), 
                             device_name.c_str());
            int rv = zmsg_addmsg (ret, &el);
            assert ( rv != -1 );
            assert ( el == NULL );
        } // end for
        log_info("end select devices\n"); 
    }
    catch (const std::exception &e) {
        // internal error in database
        zlist_destroy (&powers);
        zmsg_destroy  (&ret);
        log_warning ("abort with err = '%s'\n", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
    }

    zframe_t* devices = NULL;
    int rv = matryoshka2frame (&ret, &devices);
    assert ( rv == 0 );
    zmsg_t* result = asset_msg_encode_return_power (devices, powers);
    zlist_destroy  (&powers);
    zframe_destroy (&devices);
    zmsg_destroy   (&ret);
    log_info ("end normal\n");
    return result;
}

/*
 * \brief This function processes the ASSET_MSG_GET_POWER_DATACENTER message
 *
 * In case of success it generates the ASSET_MSG_RETURN_POWER. 
 * In case of failure returns COMMON_MSG_FAIL.
 * 
 * Returns all devices in datacenter and all powerlinks between them.
 * Links outside the datacenter are not returned.
 *
 * A single powerchain link is coded as "A:B:C:D" string 
 * ("src_socket:src_id:dst_socket:dst_id").
 * If A or C is zero then A or C were not srecified in database (were NULL). 
 *
 * \param url    - the connection to database.
 * \param getmsg - the message of the type ASSET_MSG_GET_POWER_DATACENTER
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       ASSET_MSG_RETURN_DATACENTER
 */ 
zmsg_t* get_return_power_topology_datacenter(const char* url, 
                                    asset_msg_t* getmsg)
{
    assert ( getmsg );
    assert ( asset_msg_id (getmsg) == ASSET_MSG_GET_POWER_DATACENTER );
    log_info ("start\n");
    uint32_t element_id   = asset_msg_element_id  (getmsg);
    uint8_t  linktype = INPUT_POWER_CHAIN;

    // devices
    zmsg_t* ret = zmsg_new();
    assert ( ret );

    try{
        tntdb::Connection conn = tntdb::connectCached(url);
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            " v.id_asset_element, v.name , v.type_name" 
            " FROM"
            "   v_bios_asset_element_super_parent v"
            " WHERE :dcid IN (v.id_parent1, v.id_parent2 ,v.id_parent3,"
            "                   v.id_parent4)"
        );
        // can return more than one row
        tntdb::Result result = st.setInt("dcid", element_id).
                                  select();
        
        for ( auto &row: result )
        {
            // id_asset_element, required
            uint32_t id_asset_element = 0;
            row[0].get(id_asset_element);
            assert ( id_asset_element );
            
            // device_name, required
            std::string device_name = "";
            row[1].get(device_name);
            assert ( device_name != "" );
            
            // device_type_name, required 
            std::string device_type_name = "";
            row[2].get(device_type_name);
            assert ( device_type_name != "" );

            log_debug ("for\n");
            log_debug ("device_name = %s\n", device_name.c_str());
            log_debug ("device_type_name = %s\n", device_name.c_str());
            log_debug ("asset_element_id = %d\n", id_asset_element);
            zmsg_t* el = asset_msg_encode_powerchain_device
                            (id_asset_element, device_type_name.c_str(), 
                             device_name.c_str());
            int rv = zmsg_addmsg (ret, &el);
            assert ( rv != -1 );
            assert ( el == NULL );
        } // end for
    }
    catch (const std::exception &e) {
        // internal error in database
        zmsg_destroy (&ret);
        log_warning ("abort with err = '%s'\n", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
    }
    // device is ok

    // powers
    zlist_t* powers = zlist_new();
    assert ( powers );
    zlist_set_duplicator (powers, void_dup);
    try{
        tntdb::Connection conn = tntdb::connectCached(url);
        // v_bios_asset_link are only devices, 
        // so there is no need to add more constrains
        log_info ("start select \n");
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
        tntdb::Result result = st.setInt("dcid", element_id).
                                  setInt("linktypeid", linktype).
                                  select();
        
        log_info ("end select \n");
        char buff[28];     // 10+3+3+10+ safe 2
        
        // Go through the selected links
        for ( auto &row: result )
        {
            // src_out 
            uint32_t src_out = 0;
            row[0].get(src_out);
            
            // id_asset_element_src, required
            uint32_t id_asset_element_src = 0;
            row[1].get(id_asset_element_src);
            assert ( id_asset_element_src );
            
            // dest_in
            uint32_t dest_in = 0;
            row[2].get(dest_in);
            
            // id_asset_element_dest, required
            uint32_t id_asset_element_dest = 0;
            row[3].get(id_asset_element_dest);
            assert ( id_asset_element_dest );
            
            log_debug ("for\n");
            log_debug ("asset_element_id_src = %d\n", id_asset_element_src);
            log_debug ("asset_element_id_dest = %d\n", id_asset_element_dest);
            log_debug ("src_out = %d\n", src_out);
            log_debug ("dest_in = %d\n", dest_in);

            sprintf(buff, "%d:%d:%d:%d", src_out, id_asset_element_src, 
                                            dest_in, id_asset_element_dest);
            zlist_push (powers, buff);
        } // end for
    }
    catch (const std::exception &e) {
        // internal error in database
        zlist_destroy (&powers);
        log_warning ("abort with err = '%s'\n", e.what());
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
    }
    // powers is ok

    zframe_t* devices = NULL;
    int rv = matryoshka2frame (&ret, &devices);
    assert ( rv == 0 );
    zmsg_t* result = asset_msg_encode_return_power (devices, powers);
    zlist_destroy  (&powers);
    zframe_destroy (&devices);
    zmsg_destroy   (&ret);
    log_info ("end normal\n");
    return result;
}
