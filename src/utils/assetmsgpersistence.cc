// returns: asset_msg_fail error, success - return_* / ok, in case of NULL means that nothing to send in response
// if some id is 0 it means that in database there was a NULL-value
// assumption: every ID is unsigned
// Hash values must be printable strings; keys may not contain '='.
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


//Function responsible for creating the ASSET_MSG_ELEMENT
//Function responsible for creating hashmap for the ext attributes
//Function responsible for creating the ASSET_MSG_DEVICE
//Function responsible for creating the "strings" of groups
//Function responsible for creating the "strings" of powers

//Function process the get_element message
// element_found                element_not_found           internal_error   
// ASSET_MSG_RETURN_ELEMENT     ASSET_MSG_FAIL              ASSET_MSG_FAIL
// with msg = msg               DB_ERROR_NOTFOUND           DB_ERROR_INTERNAL  
asset_msg_t* _get_asset_element(const char *url, asset_msg_t *msg)
{
    assert(msg);

    const unsigned int element_id      = asset_msg_element_id (msg); 
    const unsigned int element_type_id = asset_msg_type (msg);

    tntdb::Connection conn; 
    conn = tntdb::connectCached(url);

    // Can return one row or nothing.
    // Get basic attributes of the element
    tntdb::Statement st = conn.prepareCached(
        " select"
        " v.name , v.id_parent, v.id_parent_type"
        " from"
        " v_bios_asset_element v"
        " where v.id = :id and v.id_type = :typeid"
        );
    
    asset_msg_t *resultmsg = NULL;
    
    tntdb::Row row;
    try {
        row = st.setInt("id", element_id).
                 setInt("typeid",element_type_id).
                 selectRow();
    }
    catch (const tntdb::NotFound &e) {
        // element with specified type was not found
        resultmsg = asset_msg_new (ASSET_MSG_FAIL);
        assert(resultmsg);
    
        asset_msg_set_error_id (resultmsg, DB_ERROR_NOTFOUND);
        return resultmsg;
    }
    catch (const std::exception &e) {
        // internal error in database 
        resultmsg = asset_msg_new (ASSET_MSG_FAIL);
        assert(resultmsg);

        asset_msg_set_error_id (resultmsg, DB_ERROR_INTERNAL);
        return resultmsg;
    }

    // element was found
    // name, is required
    std::string name = "";
    row[0].get(name);
    assert(name != "");  //database is corrupted

    // parent_id, is not required
    unsigned int parent_id = 0;
    row[1].get(parent_id);

    // parent_type_id, required, if parent_id != 0
    unsigned int parent_type_id = 0;
    row[2].get(parent_type_id);
    assert( ! ( ( parent_type_id == 0 ) && (parent_id != 0) ) ); // database is corrupted
       
    // Get extra attributes of the element
    // Can return more than one row
    tntdb::Statement st_extattr = conn.prepareCached(
        " select"
        " v.keytag , v.value"
        " from"
        " v_bios_asset_ext_attributes v"
        " where v.id_asset_element = :idelement"
        );

    tntdb::Result result;  
    try {
        result = st_extattr.setInt("idelement", element_id).
                            select();
    }
    catch (const std::exception &e) {
        // internal error in database
        resultmsg = asset_msg_new (ASSET_MSG_FAIL);
        assert(resultmsg);

        asset_msg_set_error_id (resultmsg, DB_ERROR_INTERNAL);
        return resultmsg;
    }

    zhash_t *extAttributes = zhash_new();
    assert(extAttributes);

    // Go through the selected extra attributes
    for ( tntdb::Result::const_iterator it = result.begin();
            it != result.end(); ++it)
    {
        tntdb::Row row = *it;

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
    
    asset_msg_t* msgelement = asset_msg_new (ASSET_MSG_ELEMENT);
    assert(msgelement);
    asset_msg_set_name(msgelement, name.c_str());
    asset_msg_set_location(msgelement,parent_id);
    asset_msg_set_location_type(msgelement,parent_type_id);
    asset_msg_set_type(msgelement,element_type_id);    
    asset_msg_set_ext(msgelement, &extAttributes);
    
    asset_msg_t *msgdevice = NULL;
    if ( element_type_id == asset_type::DEVICE )
    {
        // Get more attributes of the device
        // Can return one row or nothing 
        tntdb::Statement st_dev = conn.prepareCached(
            " select"
            " conv(v.mac,10,16) , v.ip, v.hostname , v.full_hostname , v.id_asset_device_type"
            " from"
            " v_bios_asset_device v"
            " where v.id_asset_element = :idelement"
        );

        try {
            row = st_dev.setInt("idelement", element_id).
                         selectRow();
        }
        catch (const tntdb::NotFound &e) {
            assert(false);      
            //database is corrupted, for every device in db there should be two rows
            //1 in asset_element
            //2 in asset_device
        }
        catch (const std::exception &e) {
            // internal error in database
            resultmsg = asset_msg_new (ASSET_MSG_FAIL);
            assert(resultmsg);
    
            asset_msg_set_error_id (resultmsg, DB_ERROR_INTERNAL);
            return resultmsg;
        }
       
        // TODO: Assumption: if data where inserted in database, then assume they are corrected
        // mac in db is stored as a number, and without :
        std::string mac = "";
        row[0].get(mac);
        if ( mac != "" )
           mac = _addColonMacaddress(mac);

        // ip
        std::string ip = "";
        row[1].get(ip);

        // hostname
        std::string hostname = "";
        row[2].get(hostname);

        // fullhostname
        std::string fullhostname = "";
        row[3].get(fullhostname);

        // id_asset_device_type
        unsigned int id_asset_device_type = 0;
        row[4].get(id_asset_device_type);
        assert( id_asset_device_type != 0);  //database is corrupted
        
        // GROUPS
        // Get information about the groups device belongs to
        // Can return more than one row
        tntdb::Statement st_gr = conn.prepareCached(
            " select"
            " v.id_asset_group"
            " from"
            " v_bios_asset_group_relation v"
            " where v.id_asset_element = :idelement"
        );
 
        try {
            result = st_gr.setInt("idelement", element_id).
                           select();
        }
        catch (const std::exception &e) {
            // internal error in database
            resultmsg = asset_msg_new (ASSET_MSG_FAIL);
            assert(resultmsg);
    
            asset_msg_set_error_id (resultmsg, DB_ERROR_INTERNAL);
            return resultmsg;
        }
        

        zlist_t *groups = zlist_new();
        assert(groups);
        // TODO look at the possibility to add elements dirictly !!!
        // Go through the selected groups
        for ( tntdb::Result::const_iterator it = result.begin();
                it != result.end(); ++it)
        {
            tntdb::Row row = *it;

            // group_id, required
            unsigned int group_id = 0;
            row[0].get(group_id);
            assert( group_id != 0 );  //database is corrupted

            static char buff[16];
            sprintf(buff, "%d", group_id);
            zlist_push( groups, buff );
        }

        // POWERS
        // Get information about the power chains device belongs to
        // Can return more than one row
        tntdb::Statement st_pow = conn.prepareCached(
            " select"
            " v.id_asset_device_src , v.src_out , v.dest_in"
            " from"
            " v_bios_asset_link v"
            " where v.id_asset_device_dest = :idelement and v.id_asset_link_type = 1"
        );  // TODO link type now 1 means POWER
 
        try {
            result = st_pow.setInt("idelement", element_id).
                           select();
        }
        catch (const std::exception &e) {
            // internal error in database
            resultmsg = asset_msg_new (ASSET_MSG_FAIL);
            assert(resultmsg);
    
            asset_msg_set_error_id (resultmsg, DB_ERROR_INTERNAL);
            return resultmsg;
        }

        zlist_t *powers = zlist_new();
        assert(powers);

        // Go through the selected groups
        for ( tntdb::Result::const_iterator it = result.begin();
                it != result.end(); ++it)
        {
            tntdb::Row row = *it;

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

            asset_msg_t* link = asset_msg_new (ASSET_MSG_LINK);
            asset_msg_set_src_socket(link, src_out);
            asset_msg_set_dst_socket(link, dest_in);
            asset_msg_set_src_location(link, src_id);
            asset_msg_set_dst_location(link, element_id);
            
            zmsg_t* tempmsg = asset_msg_encode (&link);
            byte* buff;
            zmsg_encode(tempmsg,&buff);
            zlist_push(powers, buff );
        }

        msgdevice = asset_msg_new (ASSET_MSG_DEVICE);
        assert(msgdevice);

        asset_msg_set_ip(msgdevice, ip.c_str());
        asset_msg_set_mac(msgdevice,mac.c_str());
        asset_msg_set_hostname(msgdevice,hostname.c_str());
        asset_msg_set_fqdn(msgdevice,fullhostname.c_str());   
        static char buff[16];
        sprintf(buff, "%d", id_asset_device_type);
        asset_msg_set_device_type(msgdevice, buff);
        
        zmsg_t* nnmsg = NULL;
        nnmsg = asset_msg_encode(&msgelement);
        asset_msg_set_msg (msgdevice, &nnmsg);
        asset_msg_set_groups (msgdevice, &groups);
        asset_msg_set_powers (msgdevice, &powers);

        zmsg_destroy(&nnmsg);

        zlist_destroy(&powers);
        zlist_destroy(&groups);
        // TODO POWERS
    }
          
    //make ASSET_MSG_RETURN_ELEMENT
    resultmsg = asset_msg_new (ASSET_MSG_RETURN_ELEMENT);
    assert(resultmsg);
    asset_msg_set_element_id (resultmsg, element_id);

    zmsg_t* nmsg = NULL;
    if ( element_type_id == asset_type::DEVICE )
        nmsg = asset_msg_encode(&msgdevice);
    else
        nmsg = asset_msg_encode(&msgelement);
    asset_msg_set_msg (resultmsg,&nmsg);

    asset_msg_destroy(&msgdevice);
    zhash_destroy(&extAttributes);
    asset_msg_destroy(&msgelement);

    log_info ("%s", "get_asset_element() end\n");
    return resultmsg;
}

// element_found                element_not_found           internal_error
// ASSET_MSG_RETURN_ELEMENTS    ASSET_MSG_RETURN_ELEMENTS   ASSET_MSG_FAIL
// with filled dictionary       with empty dictionary       
asset_msg_t* _get_asset_elements(const char *url, asset_msg_t *msg)
{
    assert(msg);

    const unsigned int element_type_id = asset_msg_type (msg);
     
    tntdb::Connection conn; 
    conn = tntdb::connectCached(url);

    // Can return more than one row.
    tntdb::Statement st = conn.prepareCached(
        " select"
        " v.name , v.id"
        " from"
        " v_bios_asset_element v"
        " where v.id_type = :typeid"
        );
    
    asset_msg_t *resultmsg = NULL;
    
    tntdb::Result result; 
    try{
        result = st.setInt("typeid", element_type_id).
                    select();
    }
    catch (const std::exception &e)
    {
        // internal error in database
        resultmsg = asset_msg_new (ASSET_MSG_FAIL);
        assert(resultmsg);
        asset_msg_set_error_id (resultmsg, DB_ERROR_INTERNAL);
        return resultmsg;
    }

    if ( result.size() == 0 )
    {
        // elements were not found
        resultmsg = asset_msg_new (ASSET_MSG_FAIL);
        assert(resultmsg);
        // TODO now there is no difference between notfound and bad group 
        asset_msg_set_error_id (resultmsg, DB_ERROR_NOTFOUND);
        return resultmsg;
    }

    // elements was found
    zhash_t *elements = zhash_new();
    assert(elements);

    // Go through the selected elements
    for ( tntdb::Result::const_iterator it = result.begin();
            it != result.end(); ++it)
    {
        tntdb::Row row = *it;

        // name, is required
        std::string name = "";
        row[0].get(name);
        assert(name != "");  //database is corrupted

        // id, is required
        unsigned int id = 0;
        row[1].get(id);
        assert( id != 0);  //database is corrupted

        static char buff[16];
        sprintf(buff, "%d", id);
        zhash_insert(elements, buff, (void*)name.c_str());
    }
   
    // make ASSET_MSG_RETURN_ELEMENT
    resultmsg = asset_msg_new (ASSET_MSG_RETURN_ELEMENTS);
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

