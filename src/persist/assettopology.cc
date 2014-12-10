#include <assert.h>
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


#include "assettopology.h"
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
 * \brief Select group elements of specified type for the specified group
 *  
 * \param url             - connection to database
 * \param element_id      - asset element id of group
 * \param filter_type     - id of the type of the searched elements
 *                          (from the table t_bios_asset_element_type)
 *
 * \return zmsg_t  - in case of success returns ASEET_MSG_RETURN_LOCATION_FROM
 *                   filled with elements of the group,
 *                   or COMMON_MSG_FAIL in case of errors.
 */
zmsg_t* select_group_elements(const char* url, uint32_t element_id, 
        uint8_t element_type_id, const char* group_name, 
        const char* dtype_name, uint8_t filtertype)
{
    assert ( element_id );  // id of the group should be specified
    assert ( ( filtertype >= 1 ) && ( filtertype <= 7 ) ); 
    // it can be only 1,2,3,4,5,6.7. 7 means - take all

    log_info ("start\n");
    log_info ("element_id = %d\n", element_id);
    log_info ("filter_type = %d\n", filtertype);
 
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
        
        log_info("rows selected %d\n", result.size());
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

            uint32_t id_type = 0;
            row[2].get(id_type);
            assert ( id_type );
            
            std::string dtype_name = "";
            row[3].get(dtype_name);
            
            log_info ("for\n");
            log_info ("i = %d\n", i);
            log_info ("id = %d\n", id);
            log_info ("name = %s\n", name.c_str());
            log_info ("id_type = %d\n", id_type);
            log_info ("dtype_name = %s\n", dtype_name.c_str());

            // we are interested in the element if we are interested in all
            // elements ( filtertype == 7) or if this element has exactly the type
            // of the filter (filtertype == id_type)
            if (( filtertype == 7 ) || ( ( filtertype == 1 ) ) )
            {
                zmsg_t* el = asset_msg_encode_return_location_from 
                              (id, id_type, name.c_str(), dtype_name.c_str(), NULL, NULL, 
                                   NULL, NULL, NULL, NULL);
                assert ( el );
        
                log_info ("created msg el for i = %d\n", i);
                                // we are interested in the element
                if ( id_type == 2 )
                {
                    rv = zmsg_addmsg (dcss, &el);
                    assert ( rv != -1 );
                    assert ( el == NULL );                 
                }
                if ( id_type == 3 )
                {   
                    rv = zmsg_addmsg (roomss, &el);
                    assert ( rv != -1 );
                    assert ( el == NULL );
                }
                if ( id_type == 4 )
                {   
                    rv = zmsg_addmsg (rowss, &el);
                    assert ( rv != -1 );
                    assert ( el == NULL );
                }
                if ( id_type == 5 )
                {   
                    rv = zmsg_addmsg (rackss, &el);
                    assert ( rv != -1 );
                    assert ( el == NULL );
                }
                if ( id_type == 6 )
                {   
                    rv = zmsg_addmsg (devicess, &el);
                    assert ( rv != -1 );
                    assert ( el == NULL );
                }
               // group of groups is not allowed 
            }
        }// end for
        zframe_t* dcs     = NULL;
        zframe_t* rooms   = NULL;
        zframe_t* rows    = NULL;
        zframe_t* racks   = NULL;
        zframe_t* devices = NULL;
        
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
        zmsg_t* el = asset_msg_encode_return_location_from 
                      (element_id, element_type_id, group_name, dtype_name, dcs, rooms, 
                                        rows, racks, devices, NULL);
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
    assert ( child_type_id );   // is required
    assert ( ( filtertype >= 1 ) && ( filtertype <= 7 ) ); 
    // it can be only 1,2,3,4,5,6.7. 7 means - take all

    log_info ("start select_childs\n");
    log_info ("depth = %d\n", current_depth);
    log_info ("element_id = %d\n", element_id);
    log_info ("element_type_id = %d\n", element_type_id);
    log_info ("child_type_id = %d\n", child_type_id);

    try{
        tntdb::Connection conn = tntdb::connectCached(url); 
        tntdb::Statement st;
        tntdb::Result result; 
        if ( element_id != 0 )
        {
            // for the groups, other select is needed
            // because type of the group should be selected
            if ( child_type_id == 1 )
            {
                st = conn.prepareCached(
                    " SELECT"
                    "    v.id, v.name, v.id_type, v1.value as dtype_name"                  
                    " FROM v_bios_asset_element v"
                    "    INNER JOIN t_bios_asset_ext_attributes v1"
                    "      ON (v.id = v1.id_asset_element AND v1.keytag = 'type')"
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
            if ( child_type_id == 1 )
            {
                st = conn.prepareCached(
                    " SELECT"
                    "    v.id, v.name, v.id_type, v1.value as dtype_name"                  
                    " FROM v_bios_asset_element v"
                    "   INNER JOIN t_bios_asset_ext_attributes v1"
                    "      ON (v.id = v1.id_asset_element AND v1.keytag = 'type')"
                    " WHERE v.id_parent is NULL  AND "
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
            
            std::string dtype_name = "";
            row[3].get(dtype_name);
            
            log_info ("for\n");
            log_info ("i = %d\n", i);
            log_info ("id = %d\n", id);
            log_info ("name = %s\n", name.c_str());
            log_info ("id_type = %d\n", id_type);
            log_info ("dtype_name = %s\n", dtype_name.c_str());

            zframe_t* dcs     = NULL;
            zframe_t* rooms   = NULL;
            zframe_t* rows    = NULL;
            zframe_t* racks   = NULL;
            zframe_t* devices = NULL;
            zmsg_t* grp       = NULL;
            
            if ( ( is_recursive ) && ( child_type_id != DEVICE ) && 
                        ( current_depth <= MAX_RECURSION_DEPTH ) )
            {
                if  ( ( child_type_id != 1 ) && ( child_type_id < 2 ) && ( 2 <= filtertype ) )
                {
                    log_info ("start select_dcs\n");
                    dcs     = select_childs (url, id, child_type_id, 2, 
                                is_recursive, current_depth + 1, filtertype);
                    log_info ("end select_dcs\n");
                }
                if ( ( child_type_id != 1 ) && ( child_type_id < 3 ) && ( 3 <= filtertype ) )
                {
                    log_info ("start select_rooms\n");
                    rooms   = select_childs (url, id, child_type_id, 3, 
                                is_recursive, current_depth + 1, filtertype);
                    log_info ("end select_rooms\n");
                }
                if ( ( child_type_id != 1 ) && ( child_type_id < 4 ) && ( 4 <= filtertype ) )
                {
                    log_info ("start select_rows\n");
                    rows    = select_childs (url, id, child_type_id, 4, 
                            is_recursive, current_depth + 1, filtertype);
                    log_info ("end select_rows\n");
                }
                if ( ( child_type_id != 1 ) && ( child_type_id < 5 ) && ( 5 <= filtertype ) )
                {
                    log_info ("start select_racks\n");
                    racks   = select_childs (url, id, child_type_id, 5, 
                            is_recursive, current_depth + 1, filtertype);
                    log_info ("end select_racks\n");
                }
                if ( ( child_type_id != 1 ) && ( child_type_id < 6 ) && ( 6 <= filtertype ) )
                {
                    log_info ("start select_devices\n");
                    devices = select_childs (url, id, child_type_id, 6, 
                            is_recursive, current_depth + 1, filtertype);
                    log_info ("end select_devices\n");
                }
                
                // if it is a group, then do a special processing
                if ( ( child_type_id == 1 )  && ( ( 1 == filtertype ) || (filtertype == 7 ) ) )
                {
                    log_info ("start select elements of the grp\n");
                    grp = select_group_elements (url, id, 1, name.c_str(), dtype_name.c_str(), filtertype);
                    log_info ("end select elements of the grp\n");
                }
            }
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
                if ( ( child_type_id == 1 ) && (is_recursive ) )
                    el = grp;
                else 
                    el = asset_msg_encode_return_location_from 
                                (id, id_type, name.c_str(), dtype_name.c_str(), dcs, rooms, 
                                        rows, racks, devices, NULL);
                assert ( el );
                log_info ("created msg el for i = %d\n",i);
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
    // element_id = 0 means, that we are looking for unlocated elements
    // then recursive is false
    // we are not interested in datacenters and groups
    uint32_t element_id   = asset_msg_element_id  (getmsg);
    uint8_t  type_id      = 0;
    bool     is_recursive = false;  
    if ( element_id != 0 )
    {   
        is_recursive      = asset_msg_recursive   (getmsg);
        type_id           = asset_msg_type        (getmsg);
    }
    uint8_t  filter_type  = asset_msg_filter_type (getmsg);
    
    log_info("filter = %d\n", filter_type);
    zframe_t* dcs     = NULL;
    zframe_t* rooms   = NULL;
    zframe_t* rows    = NULL;
    zframe_t* racks   = NULL;
    zframe_t* devices = NULL;
    zframe_t* grps    = NULL;
    
    std::string name = "";
    std::string dtype_name = "";
    if ( element_id != 0 )
    {
        try{
            tntdb::Connection conn = tntdb::connectCached(url);

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

    if ( ( type_id < 2 ) && ( 2 <= filter_type ) && ( element_id != 0 ) )
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
    if  ( ( type_id == 2 )  && ( ( 1 == filter_type ) || (filter_type == 7 ) ) )
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
                        (element_id, type_id, name.c_str(), dtype_name.c_str(), dcs, rooms, 
                         rows, racks, devices, grps);
    log_info ("end normal\n");
    return el;
}

/* helper function */
bool compare_start_element (asset_msg_t* rmsg, uint32_t id, uint8_t id_type, const char* name, const char* dtype_name)
{
    if ( asset_msg_id (rmsg) != ASSET_MSG_RETURN_LOCATION_FROM )
        return false;
    else if ( ( asset_msg_element_id (rmsg) == id ) && ( asset_msg_type (rmsg) == id_type ) && ( !strcmp (asset_msg_name (rmsg), name) ) 
                && ( !strcmp (asset_msg_device_type (rmsg), dtype_name) ) )
        return true;
    else
        return false;
}

/**
 * \brief A helper function: convert a frame into the std::set of edges in the tree
 *  ASSET_MSG_LOCATION_FROM message
 *
 *  \param frame - frame to print
 */
edge_lf print_frame_to_edges (zframe_t* frame, uint32_t parent_id, int type, std::string name, std::string dtype_name)
{    
    byte* buffer = zframe_data (frame);
    assert ( buffer );
    
    edge_lf result, result1;

    zmsg_t* zmsg = zmsg_decode ( buffer, zframe_size (frame));
    assert ( zmsg );
    assert ( zmsg_is (zmsg) );
     
    zmsg_t* pop = NULL;
    while ( ( pop = zmsg_popmsg (zmsg) ) != NULL )
    { // caller owns zmgs_t
        asset_msg_t* item = asset_msg_decode (&pop); // zmsg_t is freed
        assert ( item );
//        asset_msg_print (item);
        
        result.insert(std::make_tuple (asset_msg_element_id(item), asset_msg_type(item), asset_msg_name(item), asset_msg_device_type(item) , parent_id, type, name, dtype_name)); 
        log_debug ("parent_id = %d\n", parent_id );
        
        zframe_t* fr = asset_msg_dcs (item);
        result1 = print_frame_to_edges (fr, asset_msg_element_id (item), asset_msg_type(item), asset_msg_name(item), asset_msg_device_type(item));
        result.insert(result1.begin(), result1.end());
        
        fr = asset_msg_rooms (item);
        result1 = print_frame_to_edges (fr, asset_msg_element_id (item),asset_msg_type(item), asset_msg_name(item), asset_msg_device_type(item));
        result.insert(result1.begin(), result1.end());
        
        fr = asset_msg_rows (item);
        result1 = print_frame_to_edges (fr, asset_msg_element_id (item),asset_msg_type(item), asset_msg_name(item), asset_msg_device_type(item));
        result.insert(result1.begin(), result1.end());
        
        fr = asset_msg_racks(item);
        result1 = print_frame_to_edges (fr, asset_msg_element_id (item),asset_msg_type(item), asset_msg_name(item), asset_msg_device_type(item));
        result.insert(result1.begin(), result1.end());
        
        fr = asset_msg_devices (item);
        result1 = print_frame_to_edges (fr, asset_msg_element_id (item),asset_msg_type(item), asset_msg_name(item), asset_msg_device_type(item));
        result.insert(result1.begin(), result1.end());
        
        fr = asset_msg_grps (item);
        result1 = print_frame_to_edges (fr, asset_msg_element_id (item),asset_msg_type(item), asset_msg_name(item), asset_msg_device_type(item));
        result.insert(result1.begin(), result1.end());
//            printf ("\tstatus = %d\n", (int) test_msg_status (item));
        
        asset_msg_destroy (&item);
        assert ( pop == NULL );
    }
   zmsg_destroy (&zmsg);
   assert ( zmsg == NULL );
   return result;
}


/**
 * \brief A helper function: prints the frames in the
 *  ASSET_MSG_LOCATION_FROM message
 *
 *  \param frame - frame to print
 */
void print_frame (zframe_t* frame, uint32_t parent_id)
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
        printf ("parent_id = %d\n", parent_id );
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
//            printf ("\tstatus = %d\n", (int) test_msg_status (item));
        asset_msg_destroy (&item);
        assert ( pop == NULL );
    }
   zmsg_destroy (&zmsg);
   assert ( zmsg == NULL );
}

/**
 * \brief Recursivly selects the parents of the element, until the top 
 *  unlocated element.
 *
 * And generates the ASSET_MSG_RETURN_TOPOLOGY_TO message, but in inverse 
 * order (the specified element would be on the top level, but the top 
 * location would be at the bottom level);
 *
 * \param url             - the connection to database.
 * \param element_id      - the element id
 * \param element_type_id - id of the element's type
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
            " SELECT"
            " v.id_parent, v.id_parent_type,v.name, v1.name as dtype_name"
            " FROM"
            " v_bios_asset_element v"
            " LEFT JOIN v_bios_asset_device v1"
            "      ON (v.id = v1.id_asset_element)"
            " WHERE v.id = :elementid AND "
            "       v.id_type = :elementtypeid"
        );

        // Could return one row or nothing
        tntdb::Row row = st.setInt("elementid", element_id).
                            setInt("elementtypeid", element_type_id).
                            selectRow();
    
        uint32_t parent_id = 0;
        uint32_t parent_type_id = 0;

        std::string dtype_name = ""; 
        std::string name = "";
        row[0].get(parent_id);
        row[1].get(parent_type_id);
        row[2].get(name);
        row[3].get(dtype_name);

        log_info("rows selected %d, parent_id = %d, parent_type_id = %d\n", 1,
                    parent_id, parent_type_id);
        
        if ( parent_id != 0 )
        {  
            zmsg_t* parent = select_parents (url, parent_id, parent_type_id);
            if ( is_asset_msg (parent) )
                return asset_msg_encode_return_location_to (element_id, 
                                    element_type_id, name.c_str(), dtype_name.c_str(), parent);
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
                                    element_type_id, name.c_str(), dtype_name.c_str(), zmsg_new());
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


/**
 * \brief This function processes the ASSET_MSG_GET_POWER_FROM message
 *
 * In case of success it generates the ASSET_MSG_RETURN_POWER. 
 * In case of failure returns COMMON_MSG_FAIL.
 * powerchains: A:B:C:D if A or C is zero it means, that it was not srecified in database 
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
    uint8_t  linktype = 1; //TODO hardcoded constants

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
        log_warning ("abort with err = '%s %d %s'\n", "specified element id =", 
                            element_id, " is not a device");
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_BADINPUT, 
                                        "specified element is not a device", NULL);
    }
    zlist_t* powers = zlist_new();
    assert ( powers );
    zlist_set_duplicator (powers, void_dup);

    zframe_t* devices = NULL;
 
    std::set<std::tuple<int,std::string,std::string>> resultdevices;
    // ( id,  device_name, device_type_name )
    resultdevices.insert (std::make_tuple(element_id, device_name, device_type_name));
    
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

            log_info ("for\n");
            log_info ("asset_element_id_src = %d\n", element_id);
            log_info ("asset_element_id_dest = %d\n", id_asset_element_dest);
            log_info ("src_out = %d\n", src_out);
            log_info ("dest_in = %d\n", dest_in);
            log_info ("device_name = %s\n", device_name.c_str());
            log_info ("device_type_name = %s\n", device_type_name.c_str());

            sprintf(buff, "%d:%d:%d:%d", src_out, element_id, dest_in, id_asset_element_dest);
            zlist_push(powers, buff);
            
            resultdevices.insert (std::make_tuple(id_asset_element_dest, device_name, device_type_name));
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
                                (std::get<0>(adevice), (std::get<2>(adevice)).c_str(), (std::get<1>(adevice)).c_str() );
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
 * \brief A helper function: prints the devices frame in the
 *  ASSET_MSG_POWER_FROM message
 *
 *  \param frame - frame to print
 */
void print_frame_devices (zframe_t* frame)
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
        //            printf ("\tstatus = %d\n", (int) test_msg_status (item));
        asset_msg_destroy (&item);
        assert ( pop == NULL );
    }
   zmsg_destroy (&zmsg);
   assert ( zmsg == NULL );
}

/**
 * \brief This function processes the ASSET_MSG_GET_POWER_TO message
 *
 * In case of success it generates the ASSET_MSG_RETURN_POWER. 
 * In case of failure returns COMMON_MSG_FAIL.
 * 
 * A single powerchain link is coded as "A:B:C:D" string ("src_socket:src_id:dst_socket:dst_id").
 * If A or C is zero than A or C it was not srecified in database (it was NULL). 
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
    uint8_t  linktype = 1; //TODO hardcoded constants

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
        log_warning ("abort with err = '%s %d %s'\n", "specified element id =", 
                            element_id, " is not a device");
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_BADINPUT, 
                                        "specified element is not a device", NULL);
    }

    std::set<std::tuple<int,std::string,std::string>> newdevices, resultdevices;
    auto adevice = std::make_tuple(element_id, device_name, device_type_name);   // ( id,  device_name, device_type_name )
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
    
                log_info ("for\n");
                log_info ("asset_element_id_dest = %d\n", cur_element_id);
                log_info ("asset_element_id_src = %d\n", id_asset_element_src);
                log_info ("src_out = %d\n", src_out);
                log_info ("dest_in = %d\n", dest_in);
                log_info ("device_name_src = %s\n", device_name_src.c_str());
                log_info ("device_type_name_src = %s\n", device_type_name_src.c_str());
    
                sprintf(buff, "%d:%d:%d:%d", src_out, id_asset_element_src, dest_in, cur_element_id);
                zlist_push(powers, buff);

                newdevices.insert (std::make_tuple(id_asset_element_src, device_name_src, device_type_name_src));
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
                                (std::get<0>(adevice), (std::get<2>(adevice)).c_str(), (std::get<1>(adevice)).c_str() );
        int rv = zmsg_addmsg ( ret, &el);
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
 * powerchains: A:B:C:D if A or C is zero it means, that it was not srecified in database 
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
    uint8_t  linktype = 1; //TODO hardcoded constants

    // powers
    zlist_t* powers = zlist_new();
    assert ( powers );
    zlist_set_duplicator (powers, void_dup);
    try{
        tntdb::Connection conn = tntdb::connectCached(url);
        // v_bios_asset_link are only devices, so there is no need to add more constrains
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "   v.src_out, v.id_asset_element_src, v.dest_in, v.id_asset_element_dest"
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
            
            log_info ("for\n");
            log_info ("asset_element_id_src = %d\n", id_asset_element_src);
            log_info ("asset_element_id_dest = %d\n", id_asset_element_dest);
            log_info ("src_out = %d\n", src_out);
            log_info ("dest_in = %d\n", dest_in);

            sprintf(buff, "%d:%d:%d:%d", src_out, id_asset_element_src, dest_in, id_asset_element_dest);
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
    
    // devices
    zmsg_t*   ret     = zmsg_new();
    assert ( ret );
    try{
        tntdb::Connection conn = tntdb::connectCached(url);
        // select is done from pure t_bios_asset_element, 
        // because v_bios_asset_element has unnecessary union (for parents) here
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

            log_info ("for\n");
            log_info ("device_name = %s\n", device_name.c_str());
            log_info ("device_type_name = %s\n", device_name.c_str());
            log_info ("asset_element_id = %d\n", id_asset_element);
            zmsg_t* el = asset_msg_encode_powerchain_device
                                (id_asset_element, device_type_name.c_str(), device_name.c_str());
            int rv = zmsg_addmsg ( ret, &el);
            assert ( rv != -1 );
            assert ( el == NULL );
        } // end for
    }
    catch (const std::exception &e) {
        // internal error in database
        zlist_destroy  (&powers);
        zmsg_destroy   (&ret);
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

/**
 * \brief This function processes the ASSET_MSG_GET_POWER_DATACENTER message
 *
 * In case of success it generates the ASSET_MSG_RETURN_POWER. 
 * In case of failure returns COMMON_MSG_FAIL.
 * powerchains: A:B:C:D if A or C is zero it means, that it was not srecified in database 
 *
 * \param url - the connection to database.
 * \param msg - the message of the type ASSET_MSG_GET_POWER_DATACENTER
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       ASSET_MSG_RETURN_DATACENTER
 */ 
zmsg_t* get_return_power_topology_datacenter(const char* url, asset_msg_t* getmsg)
{
    assert ( getmsg );
    assert ( asset_msg_id (getmsg) == ASSET_MSG_GET_POWER_DATACENTER );
    log_info ("start\n");
    uint32_t element_id   = asset_msg_element_id  (getmsg);
    uint8_t  linktype = 1; //TODO hardcoded constants

    // devices
    zmsg_t*   ret     = zmsg_new();
    assert ( ret );

    try{
        tntdb::Connection conn = tntdb::connectCached(url);
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            " v.id_asset_element, v.name , v.type_name" 
            " FROM"
            "   v_bios_asset_element_super_parent v"
            " WHERE :dcid IN (v.id_parent1, v.id_parent2 ,v.id_parent3 ,v.id_parent4)"
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

            log_info ("for\n");
            log_info ("device_name = %s\n", device_name.c_str());
            log_info ("device_type_name = %s\n", device_name.c_str());
            log_info ("asset_element_id = %d\n", id_asset_element);
            zmsg_t* el = asset_msg_encode_powerchain_device
                                (id_asset_element, device_type_name.c_str(), device_name.c_str());
            int rv = zmsg_addmsg (ret, &el);
            assert ( rv != -1 );
            assert ( el == NULL );
        } // end for
    }
    catch (const std::exception &e) {
        // internal error in database
        zmsg_destroy   (&ret);
        log_warning ("1 abort with err = '%s'\n", e.what());
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
        // v_bios_asset_link are only devices, so there is no need to add more constrains
        log_info ("start select \n");
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "   v.src_out, v.id_asset_element_src, v.dest_in, v.id_asset_element_dest"
            " FROM"
            "   v_bios_asset_link v"
            " WHERE"
            "       v.id_asset_link_type = :linktypeid AND"
            "       v.id_asset_element_src IN ("
            "                 SELECT"
            "                   v.id_asset_element" 
            "                 FROM"
            "                   v_bios_asset_element_super_parent v"
            "                 WHERE"
            "                   :dcid IN (v.id_parent1, v.id_parent2 ,v.id_parent3 ,v.id_parent4)"
            "             )"
            "   AND"
            "       v.id_asset_element_dest IN ("
            "               SELECT"
            "                   v.id_asset_element" 
            "                 FROM"
            "                   v_bios_asset_element_super_parent v"
            "                 WHERE"
            "                   :dcid IN (v.id_parent1, v.id_parent2 ,v.id_parent3 ,v.id_parent4)"
            "             )"
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
            
            log_info ("for\n");
            log_info ("asset_element_id_src = %d\n", id_asset_element_src);
            log_info ("asset_element_id_dest = %d\n", id_asset_element_dest);
            log_info ("src_out = %d\n", src_out);
            log_info ("dest_in = %d\n", dest_in);

            sprintf(buff, "%d:%d:%d:%d", src_out, id_asset_element_src, dest_in, id_asset_element_dest);
            zlist_push (powers, buff);
        } // end for
    }
    catch (const std::exception &e) {
        // TODO noramal behavior 
        // internal error in database
        // zlist_destroy (&powers);
        // log_warning ("2 abort with err = '%s'\n", e.what());
        // return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
        //                                                e.what(), NULL);
        // ACE: "MARIA_DB_CRASH" workaround:
        // known crash of maria db, when result is empty, just continue
        log_warning (" links are empty  = '%s'\n", e.what());
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

