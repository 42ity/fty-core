// returns: asset_msg_fail error, success - return_* / ok, in case of NULL means that nothing to send in response
// if some id is 0 it means that in database there was a NULL-value
// assumption: every ID is unsigned
// Hash values must be printable strings; keys may not contain '='.
// The PROCESS of processing the messages on the database site is:
// Return: Message, that could be sent as a reply. But the persistence logic should decide, if it should be send or not.

#include <exception>
#include <assert.h>

#include <czmq.h>
#include <tntdb/connect.h>
#include <tntdb/row.h>
#include <tntdb/result.h>
#include <tntdb/error.h>

#include "log.h"
#include "assetmsgpersistence.h"
#include "asset_types.h"


// definition of internal functions, that are in charge of processing the specific type of the message
asset_msg_t* _get_asset_elements(const char *url, asset_msg_t *msg);
asset_msg_t* _get_asset_element(const char *url, asset_msg_t *msg);

// duplicates the items for zlist/zhash
void* void_dup(const void* a) { return strdup((char*)a); }
// different helpers
void _removeColonMacaddress(std::string &newmac);
const std::string _addColonMacaddress(const std::string &mac);
bool _checkMacaddress (const std::string &mac_address);

/**
 * \brief This function is a general function to process 
 * the asset_msg_t message
 */
asset_msg_t* asset_msg_process(const char *url, asset_msg_t *msg)
{
    log_open();
    log_set_level(LOG_DEBUG);
    log_set_syslog_level(LOG_DEBUG);
    log_info ("%s", "start\n");

    asset_msg_t *result = NULL;

    int msg_id = asset_msg_id (msg);
    
    switch (msg_id) {

        case ASSET_MSG_GET_ELEMENT:
        {   //datacenter/room/row/rack/group/device
            result = _get_asset_element(url,msg);
            break;
        }
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
            result = _get_asset_elements(url,msg);
            break;
        }
        case ASSET_MSG_ELEMENT:
        case ASSET_MSG_RETURN_ELEMENT:
        case ASSET_MSG_OK:
        case ASSET_MSG_FAIL:
        case ASSET_MSG_RETURN_ELEMENTS:
            //these messages are not recived by persistence
            //forget about it
            break;;
        default:
        {
        // Example: Let's suppose we are listening on a ROUTER socket from a range of producers.
        //          Someone sends us a message from older protocol that has been dropped.
        //          Are we going to return 'false', that usually means db fatal error, and
        //          make the caller crash/quit? OR does it make more sense to say, OK, message
        //          has been processed and we'll log a warning about unexpected message type
                  
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
 * \param element_id - the id of the element (from t_bios_asset_element) we search groups for.
 *
 * \return NULL                   if internal database error occurs.
 *         empty object zlist_t   if the specified element doesn't belong to any group.
 *         filled object zlist_t  if the specified element belongs to some groups.
 */
zlist_t* _select_asset_element_groups(const char* url, unsigned int element_id)
{
    zlist_t* groups;
    try {
        tntdb::Connection conn = tntdb::connectCached(url);

        // Get information about the groups element belongs to
        // Can return more than one row
        tntdb::Statement st_gr = conn.prepareCached(
            " select"
            " v.id_asset_group"
            " from"
            " v_bios_asset_group_relation v"
            " where v.id_asset_element = :idelement"
        );
    
        tntdb::Result result = st_gr.setInt("idelement", element_id).
                                     select(); 
        groups = zlist_new();
        assert(groups);
        zlist_set_duplicator(groups, void_dup);
    
        // Go through the selected groups
        for ( auto &row: result )
        {
            // group_id, required
            unsigned int group_id = 0;
            row[0].get(group_id);
            assert( group_id != 0 );  // database is corrupted

            char buff[16];
            sprintf(buff, "%d", group_id);
            zlist_push( groups, buff );
        }
    }
    catch (const std::exception &e) {
        // internal error in database
        return NULL;
    }
    return groups;
}


// TODO linktype = 1 means that it is a power chain
/**
 * \brief Gets data about links for the specifeid device.
 *
 * Get a list of links the device belongs to.
 * The every element of the list would be an encoded ASSET_MSG_LINK message.
 *
 * \param url          - the connection to database.
 * \param device_id    - the id of the device (from t_bios_asset_device) we search links for.
 * \param link_type_id - the type of the link (TODO specify the location)
 *
 * \return NULL                   if internal database error occurs.
 *         empty object zlist_t   if the specified device doesn't belong to any link.
 *         filled object zlist_t  if the specified device belongs to some link.
 */
zlist_t* _select_asset_device_link(const char* url, unsigned int device_id, unsigned int link_type_id)
{
    log_info("%s \n","start");
    zlist_t *links = NULL;
    try {
        tntdb::Connection conn = tntdb::connectCached(url);

        // Get information about the links the specified device belongs to
        // Can return more than one row
        tntdb::Statement st_pow = conn.prepareCached(
            " select"
            " v.id_asset_device_src , v.src_out , v.dest_in"
            " from"
            " v_bios_asset_link v"
            " where v.id_asset_device_dest = :iddevice and v.id_asset_link_type = :idlinktype"
        ); 
    
        tntdb::Result result = st_pow.setInt("iddevice", device_id).
                                      setInt("idlinktype",link_type_id).
                                      select();
        links = zlist_new();
        assert(links);
        zlist_set_duplicator(links, void_dup);
        char buff[512];

        // Go through the selected links
        for ( auto &row: result )
        {
            // src_out
            unsigned int src_out = 0;
            row[1].get(src_out);
            
            // dest_in
            unsigned int dest_in = 0;
            row[2].get(dest_in);

            // src_id, required
            unsigned int src_id = 0;
            row[0].get(src_id);
            assert( src_id != 0 );  //database is corrupted

            sprintf(buff, "%d:%d:%d:%d", src_out, src_id, dest_in, device_id);
            zlist_push(links, buff);
        }
        
    }
    catch (const std::exception &e) {
        // internal error in database
        return NULL;
    }
    log_info("%s \n","end");
    return links;
}

/**
 * \brief Generates an ASSET_MSG_FAIL message with a specified code
 *
 * \param errorid - an id of the error
 *
 * \return an asset_msg_t message of the type ASSET_MSG_FAIL with the specified error number
 *
 * TODO the codes are now defined as define. May be neee to have enum
 */
asset_msg_t* generate_fail(unsigned int errorid)
{
    asset_msg_t* resultmsg = asset_msg_new (ASSET_MSG_FAIL);
    assert(resultmsg);
    asset_msg_set_error_id (resultmsg, errorid);
    return resultmsg;
}

/**
 * \brief Gets extra attributes for the specifeid element.
 *
 * Get a hash table of key-values for existing extra attributes for the specified device.
 *
 * \param url        - the connection to database.
 * \param element_id - the id of the element (from t_bios_asset_element) we search extra attributes for.
 *
 * \return NULL                   if internal database error occurs.
 *         empty object zhash_t   if there is no any specified extra attribute.
 *         filled object zhash_t  if there is at least one exts attribute for the specified element.
 */
zhash_t* _select_asset_element_attributes(const char* url, unsigned int element_id)
{
    zhash_t *extAttributes = NULL;
    try {
        tntdb::Connection conn = tntdb::connectCached(url);
    
        // Can return more than one row
        tntdb::Statement st_extattr = conn.prepareCached(
            " select"
            " v.keytag , v.value"
            " from"
            " v_bios_asset_ext_attributes v"
            " where v.id_asset_element = :idelement"
        );

        tntdb::Result result = st_extattr.setInt("idelement", element_id).
                                          select();
        
        extAttributes = zhash_new();
        assert(extAttributes);
        // in older versions this function is called zhash_set_item_duplicator
        zhash_set_duplicator(extAttributes, void_dup);

        // Go through the selected extra attributes
        for (  auto &row: result )
        {
            // keytag, required
            std::string keytag = "";
            row[0].get(keytag);
            assert( keytag != "");  //database is corrupted

            // value , required
            std::string value = "";
            row[1].get(value);
            assert( value != "");  //database is corrupted

            zhash_insert(extAttributes, &keytag, &value );
        }
    }
    catch (const std::exception &e) {
        // internal error in database
        return NULL;
    }
    return extAttributes;
}

/**
 * \brief Gets additional information about specified device.
 *
 * Converts the the message of the type ASSET_MSG_ELEMENT to ASSET_MSG_DEVICE. 
 * In case of failure returns ASSET_MSG_FAIL.
 * 
 * \param url     - the connection to database.
 * \param element - the message of the type ASSET_MSG_ELEMENT we would like to extend.
 *
 * \return an asset_msg_t message of the type ASSET_MSG_FAIL in case of any error.
 *         an asset_msg_t message of the type ASSET_MSG_DEVICE in case of success.
 *         NULL if the passed message is not a message of the type ASSET_MSG_ELEMENT
 */
asset_msg_t* _select_asset_device(const char* url, asset_msg_t** element)
{
    log_info ("%s", "start\n");
    int msgelement_id = asset_msg_id (*element);
    if ( msgelement_id != ASSET_MSG_ELEMENT )
        return NULL;

    std::string mac = "";
    std::string ip = "";
    std::string hostname = "";
    std::string fullhostname = "";
    unsigned int id_asset_device_type = 0;
    unsigned int device_id = 0;
    zlist_t* groups = NULL;
    zlist_t* powers = NULL;
    try {
        tntdb::Connection conn = tntdb::connectCached(url);
    
        // Get more attributes of the device
        // Can return one row or nothing 
        tntdb::Statement st_dev = conn.prepareCached(
            " select"
            " conv(v.mac,10,16) , v.ip, v.hostname , v.full_hostname , v.id_asset_device_type, v.id_asset_device"
            " from"
            " v_bios_asset_device v"
            " where v.id_asset_element = :idelement"
        );
    
        unsigned int element_id = asset_msg_element_id(*element);
        tntdb::Row row = st_dev.setInt("idelement", element_id).
                                selectRow();

        // TODO: Assumption: if data where inserted in database, then assume they are corrected
        // mac in db is stored as a number, and without :
        row[0].get(mac);
        if ( mac != "" )
            mac = _addColonMacaddress(mac);

        // ip
        row[1].get(ip);

        // hostname
        row[2].get(hostname);

        // fullhostname
        row[3].get(fullhostname);

        // id_asset_device_type
        row[4].get(id_asset_device_type);
        assert( id_asset_device_type != 0);  // database is corrupted
        
        // id of the device
        row[5].get(device_id);
        assert( device_id != 0);  // database is corrupted

        groups = _select_asset_element_groups(url, element_id);
        if ( groups == NULL )    // internal error in database
            return generate_fail(DB_ERROR_INTERNAL);       
        
        // 1 means power chain TODO
        powers = _select_asset_device_link(url, device_id, 1);
        if ( powers == NULL )   // internal error in database
            return generate_fail(DB_ERROR_INTERNAL);
    }
    catch (const tntdb::NotFound &e) {
        //database is corrupted, for every device in db there should be two rows
        //1 in asset_element
        //2 in asset_device
        return generate_fail(DB_ERROR_BADINPUT);
    }
    catch (const std::exception &e) {
        // internal error in database
        return generate_fail(DB_ERROR_INTERNAL);
    }
    asset_msg_t* msgdevice = asset_msg_new (ASSET_MSG_DEVICE);
    assert(msgdevice);

    asset_msg_set_ip(msgdevice, ip.c_str());
    asset_msg_set_mac(msgdevice,mac.c_str());
    asset_msg_set_hostname(msgdevice,hostname.c_str());
    asset_msg_set_fqdn(msgdevice,fullhostname.c_str());   
    char buff[16];
    sprintf(buff, "%d", id_asset_device_type);
    asset_msg_set_device_type(msgdevice, buff);
     
    zmsg_t* nnmsg = NULL;
    nnmsg = asset_msg_encode(element);
    asset_msg_set_msg (msgdevice, &nnmsg);
    asset_msg_set_groups (msgdevice, &groups);
    asset_msg_set_powers (msgdevice, &powers);

    zmsg_destroy(&nnmsg);

    zlist_destroy(&powers);
    zlist_destroy(&groups);

    log_info ("%s", "start\n");
    return msgdevice;
}

/**
 * \brief Gets information about specified element (and its specified type) with extra attributes.
 *
 * \param url            - the connection to database.
 * \param element_id     - the id of the element (from t_bios_asset_element) we search for.
 * \param elemet_type_id - the id of element's type
 *
 * \return an asset_msg_t message of the type ASSET_MSG_FAIL in case of any error.
 *         an asset_msg_t message of the type ASSET_MSG_ELEMENT in case of success.
 */
asset_msg_t* _select_asset_element(const char* url, unsigned int element_id, unsigned int element_type_id)
{
    unsigned int parent_id = 0;
    unsigned int parent_type_id = 0;
    std::string name = "";

    try {
        tntdb::Connection conn = tntdb::connectCached(url);

        // Can return one row or nothing.
        // Get basic attributes of the element
        tntdb::Statement st = conn.prepareCached(
            " select"
            " v.name , v.id_parent, v.id_parent_type"
            " from"
            " v_bios_asset_element v"
            " where v.id = :id and v.id_type = :typeid"
        );
    
        tntdb::Row row = st.setInt("id", element_id).
                            setInt("typeid", element_type_id).
                            selectRow();
        
        // element was found
        // name, is required
        row[0].get(name);
        assert(name != "");  //database is corrupted

        // parent_id
        row[1].get(parent_id);

        // parent_type_id, required, if parent_id != 0
        row[2].get(parent_type_id);
        assert( ! ( ( parent_type_id == 0 ) && (parent_id != 0) ) ); // database is corrupted
    }
    catch (const tntdb::NotFound &e) {
        // element with specified type was not found
        return generate_fail (DB_ERROR_NOTFOUND);
    }
    catch (const std::exception &e) {
        // internal error in database 
        return generate_fail (DB_ERROR_INTERNAL);
    }
           
    zhash_t* extAttributes = _select_asset_element_attributes (url, element_id);
    if ( extAttributes == NULL )    // internal error in database
        return generate_fail (DB_ERROR_INTERNAL);

    asset_msg_t* msgelement = asset_msg_new (ASSET_MSG_ELEMENT);
    assert (msgelement);
    asset_msg_set_element_id (msgelement, element_id);
    asset_msg_set_name (msgelement, name.c_str());
    asset_msg_set_location (msgelement, parent_id);
    asset_msg_set_location_type (msgelement, parent_type_id);
    asset_msg_set_type (msgelement, element_type_id);    
    asset_msg_set_ext (msgelement, &extAttributes);

    return msgelement;
}

/**
 * \brief This function processes the ASSET_MSG_GET_ELEMENT message
 *
 * In case of success generates the ASSET_MSG_RETURN_ELEMENT. 
 * In case of failure returns ASSET_MSG_FAIL.
 * 
 * \param url - the connection to database.
 * \param msg - the message of the type ASSET_MSG_ELEMENT we would like to process.
 *
 * \return an asset_msg_t message of the type ASSET_MSG_FAIL in case of any error.
 *         an asset_msg_t message of the type ASSET_MSG_DEVICE in case of success.
 *         NULL if the passed message is not a message of the type ASSET_MSG_GET_ELEMENT
 */
asset_msg_t* _get_asset_element(const char *url, asset_msg_t *msg)
{
    log_info ("%s", "start\n");
    assert(msg);
    if ( asset_msg_id (msg) != ASSET_MSG_GET_ELEMENT )
        return NULL;

    const unsigned int element_id      = asset_msg_element_id (msg); 
    const unsigned int element_type_id = asset_msg_type (msg);
      
    asset_msg_t* msgelement = _select_asset_element (url, element_id, element_type_id);
    const unsigned int msgelement_id = asset_msg_id (msgelement);
    if ( msgelement_id == ASSET_MSG_FAIL )  // element was not found  or error occurs
        return msgelement;
    // element was found
    asset_msg_t* msgdevice = NULL;
    if ( element_type_id == asset_type::DEVICE )
    {
        msgdevice = _select_asset_device(url, &msgelement);
        assert (msgdevice);
        if ( asset_msg_id (msgdevice) == ASSET_MSG_FAIL )
            return msgdevice;
        // device was found
    }
          
    //make ASSET_MSG_RETURN_ELEMENT
    asset_msg_t* resultmsg = asset_msg_new (ASSET_MSG_RETURN_ELEMENT);
    assert (resultmsg);
    asset_msg_set_element_id (resultmsg, element_id);

    zmsg_t* nmsg = NULL;
    if ( element_type_id == asset_type::DEVICE )
        nmsg = asset_msg_encode (&msgdevice);
    else
        nmsg = asset_msg_encode (&msgelement);
    assert (nmsg);
    asset_msg_set_msg (resultmsg, &nmsg);

    asset_msg_destroy (&msgdevice);
    asset_msg_destroy (&msgelement);

    log_info ("%s", "end\n");
    return resultmsg;
}


// element_found                element_not_found           internal_error
// ASSET_MSG_RETURN_ELEMENTS    ASSET_MSG_RETURN_ELEMENTS   ASSET_MSG_FAIL
// with filled dictionary       with empty dictionary       
asset_msg_t* _get_asset_elements(const char *url, asset_msg_t *msg)
{
    assert(msg);
    zhash_t *elements = NULL;
    try{
        const unsigned int element_type_id = asset_msg_type (msg);
     
        tntdb::Connection conn = tntdb::connectCached(url);

        // Can return more than one row.
        tntdb::Statement st = conn.prepareCached(
            " select"
            " v.name, v.id"
            " from"
            " v_bios_asset_element v"
            " where v.id_type = :typeid"
        );
    
        tntdb::Result result = st.setInt("typeid", element_type_id).
                    select();
        if ( result.size() == 0 )  // elements were not found
            return generate_fail(DB_ERROR_NOTFOUND);

        // elements was found
        elements = zhash_new();
        assert(elements);
        // By default items are not duplicated
        zhash_set_duplicator(elements, void_dup);

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
        return generate_fail(DB_ERROR_INTERNAL);
    }
  
    // make ASSET_MSG_RETURN_ELEMENT
    asset_msg_t *resultmsg = asset_msg_new (ASSET_MSG_RETURN_ELEMENTS);
    assert(resultmsg);
    asset_msg_set_element_ids (resultmsg, &elements);

    zhash_destroy(&elements);
    return resultmsg;
}

// TODO: These functions are duplicate functions from nethistory.cc
// it is done, because we plan to get rid of classes

/*Internal function for remove colons from mac address
void
_removeColonMacaddress(std::string &newmac)
{
    newmac.erase (std::remove (newmac.begin(), newmac.end(), ':'), newmac.end()); 
}
*/
//Internal function for add colons to mac address
const std::string
_addColonMacaddress(const std::string &mac)
{
    std::string macWithColons(mac);
    macWithColons.insert(2,1,':');
    macWithColons.insert(5,1,':');
    macWithColons.insert(8,1,':');
    macWithColons.insert(11,1,':');
    macWithColons.insert(14,1,':');
    return macWithColons;
}

/*Internal method:check whether the mac address has right format
bool
_checkMacaddress (const std::string &mac_address)
{
    cxxtools::Regex regex("^[0-9,a-f,A-F][0-9,a-f,A-F]:[0-9,a-f,A-F][0-9,a-f,A-F]:[0-9,a-f,A-F][0-9,a-f,A-F]:[0-9,a-f,A-F][0-9,a-f,A-F]:[0-9,a-f,A-F][0-9,a-f,A-F]:[0-9,a-f,A-F][0-9,a-f,A-F]$");
    if(!regex.match(mac_address))
        return false;
    else
        return true;
}
*/

