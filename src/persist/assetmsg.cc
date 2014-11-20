// returns: asset_msg_fail error, success - return_* / ok, in case of NULL means that nothing to send in response
// if some id is 0 it means that in database there was a NULL-value
// assumption: every ID is unsigned
// Hash values must be printable strings; keys may not contain '='.
// The PROCESS of processing the messages on the database site is:
// Return: Message, that could be sent as a reply. But the persistence 
// logic should decide, if it should be send or not.

#include <exception>
#include <assert.h>

#include <czmq.h>
#include <tntdb/connect.h>
#include <tntdb/row.h>
#include <tntdb/result.h>
#include <tntdb/error.h>

#include "log.h"
#include "assetmsg.h"
#include "monitor.h"
#include "asset_types.h"

// TODO: move to some common section
// duplicates the items for zlist/zhash
void* void_dup(const void* a) { return strdup((char*)a); }

/**
 * \brief This function is a general function to process 
 * the asset_msg_t message
 */
zmsg_t* asset_msg_process(const char *url, asset_msg_t *msg)
{
    log_open();
    log_set_level(LOG_DEBUG);
    log_set_syslog_level(LOG_DEBUG);
    log_info ("%s", "start\n");

    zmsg_t *result = NULL;

    int msg_id = asset_msg_id (msg);
    
    switch (msg_id) {

        case ASSET_MSG_GET_ELEMENT:
        {   //datacenter/room/row/rack/group/device
            result = get_asset_element(url, msg);
            break;
        }
/*        case ASSET_MSG_GET_LAST_MEASUREMENTS:
        {
            result = _get_last_measurements(url, msg);
            break;
        }*/
        case ASSET_MSG_UPDATE_ELEMENT:
        {           
            //not implemented yet
            break;
        }
        case ASSET_MSG_INSERT_ELEMENT:
        {
            //not implemented yet
            break;
        }
        case ASSET_MSG_DELETE_ELEMENT:
        {
            //not implemented yet
            break;
        }
        case ASSET_MSG_GET_ELEMENTS:
        {   //datacenters/rooms/rows/racks/groups
            result = get_asset_elements(url, msg);
            break;
        }
       // case ASSET_MSG_RETURN_LAST_MEASUREMENTS:
        case ASSET_MSG_ELEMENT:
        case ASSET_MSG_RETURN_ELEMENT:
        case ASSET_MSG_OK:
        case ASSET_MSG_FAIL:
        case ASSET_MSG_RETURN_ELEMENTS:
            // these messages are not recived by persistence
            // forget about it
            break;;
        default:
        {
        // Example: Let's suppose we are listening on a ROUTER socket 
        //          from a range of producers.
        //          Someone sends us a message from older protocol 
        //          that has been dropped. Are we going to return 
        //          'false', that usually means db fatal error, and
        //          make the caller crash/quit? OR does it make more 
        //          sense to say, OK, message has been processed and 
        //          we'll log a warning about unexpected message type.
                  
            log_warning ("Unexpected message type received; message id = '%d'", static_cast<int>(msg_id));        
            break;       
        }
    }
    log_info ("%s", "end\n");
    log_close ();       
    return result;
};

/**
 * \brief Gets data about groups the specifeid element belongs.
 *
 * Get only a list of group IDs the element belongs to.
 *
 * \param url        - the connection to database.
 * \param element_id - the id of the element (from 
 *                     t_bios_asset_element) we search groups for.
 *
 * \return NULL                   if internal database error occurs.
 *         empty object zlist_t   if the specified element doesn't 
 *                                belong to any group.
 *         filled object zlist_t  if the specified element belongs to 
 *                                some groups.
 */
zlist_t* select_asset_element_groups(const char* url, 
        unsigned int element_id)
{
    assert ( element_id > 0 );

    zlist_t* groups = zlist_new();
    assert ( groups );
    zlist_set_duplicator (groups, void_dup);

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
    
        tntdb::Result result = st_gr.setInt("idelement", element_id).
                                     select(); 
    
        // Go through the selected groups
        for ( auto &row: result )
        {
            // group_id, required
            unsigned int group_id = 0;
            row[0].get(group_id);
            assert ( group_id != 0 );  // database is corrupted

            char buff[16];
            sprintf(buff, "%d", group_id);
            zlist_push (groups, buff);
        }
    }
    catch (const std::exception &e) {
        // internal error in database
        zlist_destroy (&groups);
        return NULL;
    }
    return groups;
}


// TODO linktype = 1 means that it is a power chain
/**
 * \brief Gets data about links for the specifeid device.
 *
 * Get a list of links the device belongs to.
 * The every element of the list would be a string "A:B:C:D", where 
 * A - src_out
 * B - src_id
 * C - dest_in
 * D - dest_id.
 * If A or B is 0 then it means that these parameters where not specified 
 * in the database.
 * 
 * \param url          - the connection to database.
 * \param device_id    - the id of the device (from 
 *                       t_bios_asset_device) we search links for.
 * \param link_type_id - the type of the link 
 *                       (TODO specify the location)
 *
 * \return NULL                   if internal database error occurs.
 *         empty object zlist_t   if the specified device doesn't 
 *                                belong to any link.
 *         filled object zlist_t  if the specified device belongs to 
 *                                some links.
 */
zlist_t* select_asset_device_link(const char* url, 
                unsigned int device_id, unsigned int link_type_id)
{
    log_info("%s \n","start");
    
    assert ( device_id );
    assert ( link_type_id );

    zlist_t* links = zlist_new();
    assert ( links );
    zlist_set_duplicator (links, void_dup);
    
    try {
        tntdb::Connection conn = tntdb::connectCached(url);

        // Get information about the links the specified device 
        // belongs to
        // Can return more than one row
        tntdb::Statement st_pow = conn.prepareCached(
            " SELECT"
            " v.id_asset_device_src , v.src_out , v.dest_in"
            " FROM"
            " v_bios_asset_link v"
            " WHERE v.id_asset_device_dest = :iddevice "
            "       AND v.id_asset_link_type = :idlinktype"
        ); 
    
        tntdb::Result result = st_pow.setInt("iddevice", device_id).
                                      setInt("idlinktype", link_type_id).
                                      select();
        char buff[28];     // 10+3+3+10+ safe 2

        // Go through the selected links
        for ( auto &row: result )
        { 
            // src_id, required
            unsigned int src_id = 0;
            row[0].get(src_id);
            assert ( src_id != 0 );  // database is corrupted

            // src_out
            unsigned int src_out = 0;
            row[1].get(src_out);
            
            // dest_in
            unsigned int dest_in = 0;
            row[2].get(dest_in);

            sprintf(buff, "%d:%d:%d:%d", src_out, src_id, dest_in, device_id);
            zlist_push(links, buff);
        }
    }
    catch (const std::exception &e) {
        // internal error in database
        zlist_destroy (&links);
        log_info("abnormal %s \n","end");
        return NULL;
    }
    log_info("normal %s \n","end");
    return links;
}

/**
 * \brief Gets extra attributes for the specified element.
 *
 * Get a hash table of key-value for existing extra attributes for 
 * the specified device.
 *
 * \param url        - the connection to database.
 * \param element_id - the id of the element (from t_bios_asset_element) 
 *                     we search extra attributes for.
 *
 * \return NULL                   if internal database error occurs.
 *         empty object zhash_t   if there is no any specified 
 *                                extra attributes.
 *         filled object zhash_t  if there is at least one extra attribute 
 *                                for the specified element.
 */
zhash_t* select_asset_element_attributes(const char* url, 
                                          unsigned int element_id)
{
    assert ( element_id );
    zhash_t* extAttributes = zhash_new();
    assert ( extAttributes );
    // in older versions this function is called zhash_set_item_duplicator
    zhash_set_duplicator (extAttributes, void_dup);

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

        tntdb::Result result = st_extattr.setInt("idelement", element_id).
                                          select();

        // Go through the selected extra attributes
        for (  auto &row: result )
        {
            // keytag, required
            std::string keytag = "";
            row[0].get(keytag);
            assert ( keytag != "" );  // database is corrupted

            // value , required
            std::string value = "";
            row[1].get(value);
            assert ( value != "" );   // database is corrupted

            zhash_insert (extAttributes, (void*)keytag.c_str(), 
                          (void*)value.c_str());
        }
    }
    catch (const std::exception &e) {
        // internal error in database
        zhash_destroy (&extAttributes);
        return NULL;
    }
    return extAttributes;
}

/**
 * \brief Gets additional information about specified device.
 *
 * Converts the the message of the type ASSET_MSG_ELEMENT to ASSET_MSG_DEVICE
 * in case of success or in case of failure to COMMON_MSG_FAIL.
 * It destoyes the message element and creates a new zmsg_t message.
 * 
 * \param url     - the connection to database.
 * \param element - the message of the type ASSET_MSG_ELEMENT we would like 
 *                  to extend.
 *
 * \return zmsg_t - an encoded ASSET_MSG_DEVICE or COMMON_MSG_FAIL
 */
zmsg_t* select_asset_device(const char* url, asset_msg_t** element, uint32_t element_id)
{
    log_info ("%s", "start\n");
    assert ( asset_msg_id (*element) == ASSET_MSG_ELEMENT );
    std::string mac = "";
    std::string ip = "";
    std::string hostname = "";
    std::string fqdn = "";
    unsigned int id_asset_device_type = 0;
    unsigned int device_id = 0;
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
            "   , v.id_asset_device_type, v.id_asset_device, v.name"
            " FROM"
            " v_bios_asset_device v"
            " WHERE v.id_asset_element = :idelement"
        );
    
        tntdb::Row row = st_dev.setInt("idelement", element_id).
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
        
        // id of the device, required
        row[5].get(device_id);
        assert ( device_id != 0 );  // database is corrupted

        // string representation of device type
        row[6].getString(type_name);
        assert ( type_name != "" );

        groups = select_asset_element_groups(url, element_id);

        if ( groups == NULL )    // internal error in database
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, NULL, NULL);       
        
        // 1 means power chain TODO
        powers = select_asset_device_link(url, device_id, 1);
        if ( powers == NULL )   // internal error in database
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, NULL, NULL);
    }
    catch (const tntdb::NotFound &e) {
        //database is corrupted, for every device in db there should be 
        //two rows
        //1 in asset_element
        //2 in asset_device
        asset_msg_destroy (element);
        zlist_destroy (&powers);
        zlist_destroy (&groups);

        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_BADINPUT, e.what(), NULL);
    }
    catch (const std::exception &e) {
        // internal error in database
        asset_msg_destroy (element);
        zlist_destroy (&powers);
        zlist_destroy (&groups);
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, e.what(), NULL);
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

    log_info ("%s", "end\n");
    return msgdevice;
}

/**
 * \brief Gets information about specified element (and its specified type)
 * with extra attributes.
 *
 * \param url            - the connection to database.
 * \param element_id     - the id of the element (from t_bios_asset_element) 
 *                         we search for.
 * \param elemet_type_id - the id of element's type
 *
 * \return zmsg_t - an encoded ASSET_MSG_ELEMENT or COMMON_MSG_FAIL
 */
zmsg_t* select_asset_element(const char* url, unsigned int element_id, 
                                  unsigned int element_type_id)
{
    assert ( element_id );
    assert ( element_type_id );
    unsigned int parent_id = 0;
    unsigned int parent_type_id = 0;
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
    
        tntdb::Row row = st.setInt("id", element_id).
                            setInt("typeid", element_type_id).
                            selectRow();
        
        // element was found
        // name, is required
        row[0].get(name);
        assert ( name != "" );  // database is corrupted

        // parent_id
        row[1].get(parent_id);

        // parent_type_id, required, if parent_id != 0
        row[2].get(parent_type_id);
        assert ( ! ( ( parent_type_id == 0 ) && (parent_id != 0) ) ); 
        // database is corrupted
    } 
    catch (const tntdb::NotFound &e) {
        // element with specified type was not found
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        // internal error in database 
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, e.what(), NULL);
    }
           
    zhash_t* extAttributes = select_asset_element_attributes(url, element_id);
    if ( extAttributes == NULL )    // internal error in database
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, NULL, NULL);

    zmsg_t* msgelement = asset_msg_encode_element
            ( name.c_str(), parent_id, parent_type_id, element_type_id, extAttributes);
    assert ( msgelement );

    zhash_destroy (&extAttributes);
    return msgelement;
}

/**
 * \brief This function processes the ASSET_MSG_GET_ELEMENT message
 *
 * In case of success it generates the ASSET_MSG_RETURN_ELEMENT. 
 * In case of failure returns COMMON_MSG_FAIL.
 * 
 * \param url - the connection to database.
 * \param msg - the message of the type ASSET_MSG_GET_ELEMENT 
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or ASSET_MSG_RETURN_ELEMENT
 */
zmsg_t* get_asset_element(const char *url, asset_msg_t *msg)
{
    log_info ("%s", "start\n");
    assert ( msg );
    assert ( asset_msg_id (msg) == ASSET_MSG_GET_ELEMENT );

    const unsigned int element_id      = asset_msg_element_id (msg); 
    const unsigned int element_type_id = asset_msg_type (msg);
      
    zmsg_t* msgelement = 
                select_asset_element (url, element_id, element_type_id);
    
    if ( is_common_msg(msgelement) )  
        // element was not found  or error occurs
        return msgelement;
    // element was found
    if ( element_type_id == asset_type::DEVICE )
    {
        log_info ("%s\n", "start looking for device");
        // destroys msgelement
        asset_msg_t* returnelement = asset_msg_decode (&msgelement);
        msgelement = select_asset_device(url, &returnelement, element_id);
        assert ( msgelement );
        if ( is_common_msg (msgelement) )
        {
            log_info ("%s\n", "oops, error, end");
            return msgelement;
        }
        // device was found
    }
          
    // make ASSET_MSG_RETURN_ELEMENT
    zmsg_t* resultmsg = asset_msg_encode_return_element 
                    (element_id, msgelement);
    assert ( resultmsg );
    zmsg_destroy (&msgelement);
    log_info ("%s", "end\n");
    return resultmsg;
}

/**
 * \brief This function processes the ASSET_MSG_GET_ELEMENTS message
 *
 * In case of success it generates the ASSET_MSG_RETURN_ELEMENTS. 
 * In case of failure returns COMMON_MSG_FAIL.
 * 
 * \param url - the connection to database.
 * \param msg - the message of the type ASSET_MSG_GET_ELEMENTS 
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or ASSET_MSG_RETURN_ELEMENTS
 */
zmsg_t* get_asset_elements(const char *url, asset_msg_t *msg)
{
    assert ( msg );
    assert ( asset_msg_id (msg) == ASSET_MSG_GET_ELEMENTS );

    zhash_t *elements = zhash_new();
    zhash_set_duplicator (elements, void_dup);
    assert(elements);

    try{
        const unsigned int element_type_id = asset_msg_type (msg);
     
        tntdb::Connection conn = tntdb::connectCached(url);

        // Can return more than one row.
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            " v.name, v.id"
            " FROM"
            " v_bios_asset_element v"
            " WHERE v.id_type = :typeid"
        );
    
        tntdb::Result result = st.setInt("typeid", element_type_id).
                                  select();

        if ( result.size() == 0 )  // elements were not found
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_NOTFOUND, NULL, NULL);

        // Go through the selected elements
        for ( auto &row: result )
        {
            // name, is required
            std::string name = "";
            row[0].get(name);
            assert(name != "");  // database is corrupted

            // id, is required
            unsigned int id = 0;
            row[1].get(id);
            assert( id != 0);    // database is corrupted
    
            char buff[16];
            sprintf(buff, "%d", id);
            zhash_insert(elements, buff, (void*)name.c_str());
        }
    }
    catch (const std::exception &e)
    {
        // internal error in database
        zhash_destroy (&elements);
        return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, e.what(), NULL);
    }
  
    // make ASSET_MSG_RETURN_ELEMENTS
    zmsg_t *resultmsg = asset_msg_encode_return_elements (elements);
    assert(resultmsg);

    zhash_destroy (&elements);
    return resultmsg;
}

/**
 * \brief This function looks for a device_discovered connected with 
 * the specified asset_element.
 *
 * \param url              - the connection to database.
 * \param asset_element_id - the id of the asset_element.
 *
 * \return device_discovered_id - of the device connected with the 
 *                                asset_element
 *         -error number        - in case of error
 */
uint32_t convert_asset_to_monitor(const char* url, uint32_t asset_element_id)
{
    assert ( asset_element_id );
    uint32_t device_discovered_id = 0;
    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            " id_discovered_device"
            " FROM"
            " v_bios_monitor_asset_relation"
            " WHERE id_asset_element = :id"
        );

        tntdb::Value val = st.setInt("id", asset_element_id).selectValue();

        val.get(device_discovered_id);
    }
    catch (const tntdb::NotFound &e){
        return -DB_ERROR_NOTFOUND;
    }
    catch (const std::exception &e) {
        return -DB_ERROR_INTERNAL;
    }
    return device_discovered_id;
};

/**
 * \brief This function looks for a asset_element connected with 
 * the specified device_discovered.
 *
 * \param url                 - the connection to database.
 * \param device_discovered_id - the id of the device_discovered.
 *
 * \return asset_element_id - id of the asset_element connected with the 
 *                                device_discovered
 *         -error number    - in case of error
 */
uint32_t convert_monitor_to_asset(const char* url, 
                    uint32_t discovered_device_id)
{
    assert ( discovered_device_id );
    
    uint32_t asset_element_id = 0;
    try{
        tntdb::Connection conn = tntdb::connectCached(url);
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            " id_asset_element"
            " FROM"
            " v_bios_monitor_asset_relation"
            " WHERE id_discovered_device = :id"
        );

        tntdb::Value val = st.setInt("id", discovered_device_id).selectValue();

        val.get(asset_element_id);
    }
    catch (const tntdb::NotFound &e){
        return -DB_ERROR_NOTFOUND;
    }
    catch (const std::exception &e) {
        return -DB_ERROR_INTERNAL;
    }
    return asset_element_id;
};

