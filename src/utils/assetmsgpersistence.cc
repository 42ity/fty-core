// returns: asset_msg_fail error, success - return_* / ok, in case of NULL means that nothing to send in response
// if some id is -1 it means that in database there was a NULL-value
#include <czmq.h>
#include "assetmsgpersistence.h"
#include <tntdb/connect.h>
#include <tntdb/row.h>
#include <tntdb/result.h>
#include <tntdb/error.h>
#include <exception>
#include <assert.h>
#include "log.h"

#define DB_ERROR_INTERNAL 1
#define DB_ERROR_BADINPUT 2
#define DB_ERROR_NOTFOUND 3

asset_msg_t *get_asset_elements(const char *url, asset_msg_t *msg);
asset_msg_t *get_asset_element(const char *url, asset_msg_t *msg);

asset_msg_t *asset_msg_process(const char *url, asset_msg_t *msg)
{
    log_open();
    log_set_level(LOG_DEBUG);
    log_set_syslog_level(LOG_DEBUG);
    log_info ("%s", "asset_msg_save() start\n");

    asset_msg_t *result = NULL;

    int msg_id = asset_msg_id (msg);
    
    switch (msg_id) {

        case ASSET_MSG_GET_ELEMENT:
        {   //datacenter/room/row/rack/group
            result = get_asset_element(url,msg);
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
            result = get_asset_elements(url,msg);
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
    log_info ("%s", "process_message() end\n");
    log_close ();       
    return result;
};

// element_found                element_not_found           internal_error          baddata //TODO
// ASSET_MSG_RETURN_ELEMENT     ASSET_MSG_FAIL              ASSET_MSG_FAIL
// with msg = msg               DB_ERROR_NOTFOUND           DB_ERROR_INTERNAL  
asset_msg_t *get_asset_element(const char *url, asset_msg_t *msg)
{
    assert(msg);

    const int element_id      = asset_msg_element_id (msg); 
    const int element_type_id = asset_msg_type (msg);

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
    try{
        row = st.setInt("id", element_id).
                 setInt("typeid",element_type_id).
                 selectRow();
    }
    catch (const tntdb::NotFound &e){
        // element was not found
        resultmsg = asset_msg_new (ASSET_MSG_FAIL);
        assert(resultmsg);
    
        asset_msg_set_error_id (resultmsg, DB_ERROR_NOTFOUND);
        return resultmsg;
    }
    catch (std::exception &e)
    {
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

    //parent_id, is not required
    int parent_id = 0;
    row[1].get(parent_id);

    //parent_type_id, required, if parent_id != 0
    int parent_type_id = 0;
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
    try{
        result = st_extattr.setInt("idelement", element_id).
                            select();
    }
    catch (std::exception &e)
    {
        // internal error in database
        resultmsg = asset_msg_new (ASSET_MSG_FAIL);
        assert(resultmsg);

        asset_msg_set_error_id (resultmsg, DB_ERROR_INTERNAL);
        return resultmsg;
    }

    zhash_t *extAttributes = zhash_new();
    assert(extAttributes);

    // Go through the selected extraattributes
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
    asset_msg_t *msgelement = asset_msg_new (ASSET_MSG_ELEMENT);
    assert(msgelement);
    asset_msg_set_name(msgelement, name.c_str());
    asset_msg_set_location(msgelement,parent_id);
    asset_msg_set_location_type(msgelement,parent_type_id);
    asset_msg_set_type(msgelement,element_type_id);    
    asset_msg_set_ext(msgelement, &extAttributes);
    
    //make ASSET_MSG_RETURN_ELEMENT
    resultmsg = asset_msg_new (ASSET_MSG_RETURN_ELEMENT);
    assert(resultmsg);
    asset_msg_set_element_id (resultmsg, element_id);

    zmsg_t * nmsg =  asset_msg_encode(&msgelement);

    asset_msg_set_msg (resultmsg,&nmsg);

    zhash_destroy(&extAttributes);
    asset_msg_destroy(&msgelement);

    log_info ("%s", "get_asset_element() end\n");
    return resultmsg;
}

// element_found                element_not_found           internal_error
// ASSET_MSG_RETURN_ELEMENTS    ASSET_MSG_RETURN_ELEMENTS   ASSET_MSG_FAIL
// with filled dictionary       with empty dictionary       
asset_msg_t *get_asset_elements(const char *url, asset_msg_t *msg)
{
    assert(msg);

    const int element_type_id = asset_msg_type (msg);
    
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
    catch (std::exception &e)
    {
        // internal error in database
        resultmsg = asset_msg_new (ASSET_MSG_FAIL);
        assert(resultmsg);
        asset_msg_set_error_id (resultmsg, DB_ERROR_INTERNAL);
        return resultmsg;
    }

    if ( result.size()==0 )
    {
        // elementa were not found
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

        //id, is required
        int id = 0;
        row[1].get(id);
        assert( id != 0);  //database is corrupted

        zhash_insert(elements, &name, &id );
    }
   
    //make ASSET_MSG_RETURN_ELEMENT
    resultmsg = asset_msg_new (ASSET_MSG_RETURN_ELEMENTS);
    assert(resultmsg);
    asset_msg_set_element_ids (resultmsg, &elements);

    zhash_destroy(&elements);

    return resultmsg;
}
