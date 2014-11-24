#include <czmq.h>
#include <tntdb/connect.h>
#include <tntdb/row.h>
#include <tntdb/error.h>
#include <tntdb/value.h>
#include <tntdb/result.h>
#include "log.h"
#include "assetmsg.h"
#include "common_msg.h"
#include <assert.h>
// TODO HARDCODED CONSTANTS for asset device types

// TODO This parameter should be placed in configure file
// but now configure file doesn't exist. 
// So instead of it the constat would be used
#define MAX_RECURSION_DEPTH 6
#define DEVICE 6


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
 * \brief Helper function: calculate zrame_t size even for NULL value
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
 * \brief Select childs of specified type for the specified element 
 * (element_id + element_type_id). 
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
zframe_t* select_childs(const char* url, uint32_t element_id, 
    uint32_t element_type_id, uint32_t child_type_id, 
    bool is_recursive, uint32_t current_depth, uint32_t filtertype)
{
    assert ( element_id );      // is required
    assert ( element_type_id ); // is required
    assert ( child_type_id );   // is required
    assert ( current_depth >= 0 );
    assert ( ( filtertype >= 1 ) && ( filtertype <= 7 ) ); 
    // it can be only 1,2,3,4,5,6.7. 7 means - take all

    log_info ("start\n");
    log_info ("depth = %d\n", current_depth);
    log_info ("element_id = %d\n", element_id);
    log_info ("element_type_id = %d\n", element_type_id);
    log_info ("child_type_id = %d\n", child_type_id);

    try{
        tntdb::Connection conn = tntdb::connectCached(url); 

        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            " v.id, v.name, v.id_type"
            " FROM"
            " v_bios_asset_element v"
            " WHERE v.id_parent = :elementid AND "
            "       v.id_parent_type = :elementtypeid AND "
            "       v.id_type = :childtypeid"
        );

        // Could return more than one row
        tntdb::Result result = st.setInt("elementid", element_id).
                                  setInt("elementtypeid", element_type_id).
                                  setInt("childtypeid", child_type_id).
                                  select();
        log_info("rows selected %d\n", result.size());
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

            uint32_t id_type = 0;
            row[2].get(id_type);
            assert ( id_type );
            
            log_info ("for\n");
            log_info ("i = %d\n", i);
            log_info ("name = %s\n", name.c_str());
            log_info ("id_type = %d\n", id_type);

            zframe_t* dcs     = NULL;
            zframe_t* rooms   = NULL;
            zframe_t* rows    = NULL;
            zframe_t* racks   = NULL;
            zframe_t* devices = NULL;
            zframe_t* grps    = NULL;

            if ( ( is_recursive ) && ( child_type_id != DEVICE ) && 
                        ( current_depth <= MAX_RECURSION_DEPTH ) )
            {
                if  ( ( child_type_id < 2 ) && ( 2 <= filtertype ) )
                {
                    log_info ("start select_dcs\n");
                    dcs     = select_childs (url, id, child_type_id, 2, 
                                is_recursive, current_depth + 1, filtertype);
                    log_info ("end select_dcs\n");
                }
                if ( ( child_type_id < 3 ) && ( 3 <= filtertype ) )
                {
                    log_info ("start select_rooms\n");
                    rooms   = select_childs (url, id, child_type_id, 3, 
                                is_recursive, current_depth + 1, filtertype);
                    log_info ("end select_rooms\n");
                }
                if ( ( child_type_id < 4 ) && ( 4 <= filtertype ) )
                {
                    log_info ("start select_rows\n");
                    rows    = select_childs (url, id, child_type_id, 4, 
                            is_recursive, current_depth + 1, filtertype);
                    log_info ("end select_rows\n");
                }
                if ( ( child_type_id < 5 ) && ( 5 <= filtertype ) )
                {
                    log_info ("start select_racks\n");
                    racks   = select_childs (url, id, child_type_id, 5, 
                            is_recursive, current_depth + 1, filtertype);
                    log_info ("end select_racks\n");
                }
                if ( ( child_type_id < 6 ) && ( 6 <= filtertype ) )
                {
                    log_info ("start select_devices\n");
                    devices = select_childs (url, id, child_type_id, 6, 
                            is_recursive, current_depth + 1, filtertype);
                    log_info ("end select_devices\n");
                }
                if ( child_type_id < 2 )
                {
                    log_info ("start select_grps\n");
                    grps    = select_childs (url, id, child_type_id, 1, 
                            is_recursive, current_depth + 1, filtertype);
                    log_info ("end select_grps\n");
                }
            }
            if ( !( ( filtertype < 7 ) &&
                    ( ( my_size(dcs) == 0 ) && ( my_size(rooms) == 0 ) 
                        && ( my_size(rows) == 0 ) && ( my_size(racks) == 0 ) 
                        && ( my_size(devices) == 0 ) && (my_size(grps) == 0 ) 
                    ) &&
                    ( child_type_id != filtertype ) 
                  )
               )
            {
                zmsg_t* el = asset_msg_encode_return_location_from 
                                (id, id_type, name.c_str(), dcs, rooms, 
                                        rows, racks, devices, grps);
                assert ( el );
                log_info ("created msg el for i = %d\n",i);
                rv = zmsg_addmsg ( ret, (zmsg_t **) &el);
                assert ( rv != -1 );
                assert ( el == NULL );
                zframe_destroy (&dcs);
                zframe_destroy (&rooms);
                zframe_destroy (&rows);
                zframe_destroy (&racks);
                zframe_destroy (&devices);
                zframe_destroy (&grps);
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
 * In case of success it generates the ASSET_MSG_RETURN_LOCATION_FROM. 
 * In case of failure returns COMMON_MSG_FAIL.
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
    assert ( asset_msg_id (getmsg) == ASSET_MSG_GET_LOCATION_FROM );
    log_info ("start\n");
    bool     is_recursive = asset_msg_recursive   (getmsg);
    uint32_t element_id   = asset_msg_element_id  (getmsg);
    uint8_t  type_id      = asset_msg_type        (getmsg);
    uint8_t  filter_type  = asset_msg_filter_type (getmsg);
    
    log_info("filter = %d\n", filter_type);
    zframe_t* dcs     = NULL;
    zframe_t* rooms   = NULL;
    zframe_t* rows    = NULL;
    zframe_t* racks   = NULL;
    zframe_t* devices = NULL;
    zframe_t* grps    = NULL;
    
    std::string name = "";
    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            " v.name"
            " FROM"
            " v_bios_asset_element v"
            " WHERE v.id = :id"
        );
    
        tntdb::Value val = st.setInt("id", element_id).
                              selectValue();
        val.get(name);
        assert ( strcmp(name.c_str() ,"") );
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

    if ( ( type_id < 2 ) && ( 2 <= filter_type ) )
    {
        log_info ("start select_dcs\n");
        dcs = select_childs (url, element_id, type_id, 2, is_recursive, 1, 
                                filter_type);
        if ( dcs == NULL )
        {
            log_warning ("end abnormal\n");
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        "dcs error", NULL);
        }
        log_info ("end select_dcs\n");
    }
    if ( ( type_id < 3 ) && ( 3 <= filter_type ) )
    {
        log_info ("start select_rooms\n");
        rooms = select_childs (url, element_id, type_id, 3, is_recursive, 1, 
                                filter_type);
        if ( rooms == NULL )
        {
            zframe_destroy (&dcs);
            log_warning ("end abnormal\n");
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        "rooms error", NULL);
        }
        log_info ("end select_rooms\n");
    }
    if ( ( type_id < 4 ) && ( 4 <= filter_type ) )
    {
        log_info ("start select_rows\n");
        rows = select_childs (url, element_id, type_id, 4, is_recursive, 1, 
                                filter_type);
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
    if ( ( type_id < 5 ) && ( 5 <= filter_type ) )
    {
        log_info ("start select_racks\n");
        racks = select_childs (url, element_id, type_id, 5, is_recursive, 1, 
                                filter_type);
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
    if ( ( type_id < 6 ) && ( 6 <= filter_type ) )
    {
        log_info ("start select_devices\n");
        devices = select_childs (url, element_id, type_id, 6, is_recursive, 1,
                                    filter_type );
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
    if ( type_id < 2 )
    {
        log_info ("start select_grps\n");
        grps = select_childs (url, element_id, type_id, 1, is_recursive, 1, 
                                filter_type);
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
    zmsg_t* el = asset_msg_encode_return_location_from 
                        (element_id, type_id, name.c_str(), dcs, rooms, 
                         rows, racks, devices, grps);
    log_info ("end normal\n");
    return el;
}

/**
 * \brief A helper function: prints the frames in the
 *  ASSET_MSG_LOCATION_FROM message
 *
 *  \param frame - frame to print
 */
void print_frame (zframe_t* frame)
{    
    byte* buffer = zframe_data (frame);
    assert ( buffer );

    zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );
     
    zmsg_t* pop = NULL;
    while ( ( pop = zmsg_popmsg (zmsg) ) != NULL )
    { // caller owns zmgs_t
        asset_msg_t* item = asset_msg_decode (&pop); // zmsg_t is freed
        assert ( item );
        asset_msg_print (item);
        zframe_t* fr = asset_msg_dcs (item);
        print_frame (fr);
        fr = asset_msg_rooms (item);
        print_frame (fr);
        fr = asset_msg_rows (item);
        print_frame (fr);
        fr = asset_msg_racks(item);
        print_frame (fr);
        fr = asset_msg_devices (item);
        print_frame (fr);
        fr = asset_msg_grps (item);
        print_frame (fr);
//            printf ("\tstatus = %d\n", (int) test_msg_status (item));
        asset_msg_destroy (&item);
        assert ( pop == NULL );
    }
   zmsg_destroy (&zmsg);
   assert ( zmsg == NULL );
}

/**
 * \brief Recursivly selects the parents of the lement, until the top 
 *  unlocated element.
 *
 * And generates the ASSET_MSG_RETURN_TOPOLOGY_TO message, but in inverse 
 * order (the specified element would be on the top level, but the top 
 * location would be at the bottom level;
 *
 * \param url             - the connection to database.
 * \param element_id      - the element id
 * \param element_type_id - id of the lement's type
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or ASSET_MSG_RETURN_TOPOLOGY_TO
 */
zmsg_t* select_parents (const char* url, uint32_t element_id, 
                        uint32_t element_type_id)
{
    assert ( element_id );      // is required
    assert ( element_type_id ); // is required

    log_info ("start\n");
    log_info ("element_id = %d\n", element_id);
    log_info ("element_type_id = %d\n", element_type_id);

    try{
        tntdb::Connection conn = tntdb::connectCached(url); 

        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            " v.id_parent, v.id_parent_type"
            " FROM"
            " v_bios_asset_element v"
            " WHERE v.id = :elementid AND "
            "       v.id_type = :elementtypeid"
        );

        // Could return one row or nothing
        tntdb::Row row = st.setInt("elementid", element_id).
                            setInt("elementtypeid", element_type_id).
                            selectRow();
    
        uint32_t parent_id = 0;
        uint32_t parent_type_id = 0;

        row[0].get(parent_id);
        row[1].get(parent_type_id);
        log_info("rows selected %d, parent_id = %d, parent_type_id = %d\n", 1,
                    parent_id, parent_type_id);
        
        if ( parent_id != 0 )
        {  
            zmsg_t* parent = select_parents (url, parent_id, parent_type_id);
            if ( is_asset_msg (parent) )
                return asset_msg_encode_return_location_to (element_id, 
                                    element_type_id, parent);
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
                                    element_type_id, zmsg_new());
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
    assert ( asset_msg_id (getmsg) == ASSET_MSG_GET_LOCATION_TO );
    log_info ("start\n");
    uint32_t element_id   = asset_msg_element_id (getmsg);
    uint8_t  type_id      = asset_msg_type       (getmsg);
    zmsg_t* result = select_parents (url, element_id, type_id);
    log_info ("end\n");
    return result;
}
