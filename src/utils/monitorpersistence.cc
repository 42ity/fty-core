#include <czmq.h>
#include "common_msg.h"
#include <tntdb/connect.h>
#include <tntdb/row.h>
#include <tntdb/error.h>
#include <tntdb/value.h>
#include "assetmsgpersistence.h"
#include "databasetimeobject.h"
/**
 * \brief Generates an COMMON_MSG_DB_FAIL message with a specified code
 *
 * \param errorid - an id of the error
 *
 * \return an common_msg_t message of the type COMMON_MSG_DB_FAIL with the specified error number
 *
 * TODO the codes are now defined as define. May be need to have enum
 */
common_msg_t* _generate_fail(unsigned int errorid)
{
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_DB_FAIL);
    assert(resultmsg);
    common_msg_set_errorno (resultmsg, errorid);
    return resultmsg;
}

/**
 * \brief Generates an COMMON_MSG_DB_OK message and specifies a processed id
 *
 * \param id - an id of the processed row
 *
 * \return an common_msg_t message of the type COMMON_MSG_DB_OK
 */
common_msg_t* _generate_ok(unsigned int id)
{
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_DB_OK);
    assert(resultmsg);
//    common_msg_set_id (resultmsg, id);
    return resultmsg;
}

common_msg_t* _generate_client(unsigned int id, const char* name);

common_msg_t* select_client(const char* url,const char* name)
{
    assert(strlen(name)>0);

    int id_client = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " select "
            " v.id"
            " from"
            " v_bios_client v"
            " where v.name = :name"
        );
          
        tntdb::Value val = st.setString("name", name).selectValue();
        val.get(id_client); 
    }
    catch (const tntdb::NotFound &e){
        return _generate_fail(DB_ERROR_NOTFOUND);
    }
    catch (const std::exception &e) {
        // internal error in database
        return _generate_fail(DB_ERROR_INTERNAL);
    }
    return _generate_client(id_client,name);
}

common_msg_t* select_client(const char* url, unsigned int id)
{
    std::string name = "";
    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " select "
            " v.name"
            " from"
            " v_bios_client v"
            " where v.id = :id"
        );
          
        tntdb::Value val = st.setInt("id", id).selectValue();
        val.get(name);
        assert(name != "");
    }
    catch (const tntdb::NotFound &e){
        return _generate_fail(DB_ERROR_NOTFOUND);
    }
    catch (const std::exception &e) {
        return _generate_fail(DB_ERROR_INTERNAL);
    }
    return _generate_client(id,name.c_str());
}

common_msg_t* insert_client(const char* url,const char* name)
{
    assert (strlen(name) > 0);

    unsigned int n = 0;
    unsigned int newid = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " insert into"
            " v_bios_client (id,name)"
            " values (NULL,:name)"
        );
    
        n  = st.setString("name", name).execute();
        newid = conn.lastInsertId();
    }
    catch (const std::exception &e) {
        return _generate_fail (DB_ERROR_INTERNAL);
    }
    if ( n == 1 )
        return _generate_ok (newid);
    else
        return _generate_fail (DB_ERROR_BADINPUT);
}

common_msg_t* delete_client(const char* url, unsigned int id_client)
{
    unsigned int n = 0;
    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " delete from"
            " v_bios_client "
            " where id = :id"
        );
    
        n  = st.setInt("id", id_client).execute();
    } 
    catch (const std::exception &e) {
        return _generate_fail (DB_ERROR_INTERNAL);
    }
    if ( n == 1 )
        return _generate_ok( id_client);
    else
        return _generate_fail(DB_ERROR_BADINPUT);
}

common_msg_t* update_client(const char* url, common_msg_t* client)
{
    const char* name = common_msg_name(client);
    unsigned int id = common_msg_id(client);
    unsigned int n = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " update"
            " v_bios_client"
            " set name = :name"
            " where id = :id"
        );
    
        n  = st.setString("name", name).
                setInt("id",id).
                execute();
    }
    catch (const std::exception &e) {
        return _generate_fail (DB_ERROR_INTERNAL);
    }
    if ( n == 1 )
        return _generate_ok(id);
    else
        return _generate_fail(DB_ERROR_UNKNOWN);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// client info//////////
// //////////////////////////////
common_msg_t* _generate_client_info(unsigned int id, unsigned int client_id, unsigned int device_id,time_t mytime, const char* data, unsigned int size);
/**
 * \brief Inserts into the table t_bios_client_info new row.
 *
 * \param device_id - id of the device information is about
 * \param client_id - id of the module that gathered this information
 * \param info      - an information as a flow of bytes
 * \param infolen   - the size of info
 *
 * \return COMMON_MSG_DB_FAIL if inserting failed
 *         COMMON_MSG_DB_OK   if inserting was successful
 */
common_msg_t* _insert_client_info (const char* url, unsigned int device_id, unsigned int client_id, const char* info, unsigned int infolen)
{
    assert(device_id);  // is required
    assert(client_id);  // is required
    assert(infolen);    // is required

    unsigned int n = 0; // number of rows affected
    unsigned int newid = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " insert into"
            " v_bios_client_info (id, id_client, id_discovered_device, ext, timestamp)"
            " values (NULL, :idclient, :iddiscovereddevice, :ext, NOW())"
        );          // time is the time of inserting into database
    
        tntdb::Blob blobData(info,infolen);

        n = st.setInt("idclient", client_id).
                            setInt("iddiscovereddevice",device_id).
                            setBlob("ext", blobData).
                            execute();
        newid = conn.lastInsertId();
    }
    catch (const std::exception &e) {
        return _generate_fail (DB_ERROR_INTERNAL);
    }
    if ( n == 1 )
        return _generate_ok (newid);
    else
        return _generate_fail (DB_ERROR_BADINPUT);
}

/**
 * \brief Delets from the table t_bios_client_info row by id.
 *
 * \param id - id of the row to be deleted
 *
 * \return COMMON_MSG_DB_FAIL if delete failed
 *         COMMON_MSG_DB_OK   if delete was successful
 */
common_msg_t* _delete_client_info (const char* url, unsigned int id)
{
   unsigned int n = 0;
   try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " delete from"
            " v_bios_client_info "
            " where id = :id"
        );
    
        n  = st.setInt("id", id).execute();
    }
    catch (const std::exception &e) {
        return _generate_fail (DB_ERROR_INTERNAL);
    }
    if ( n == 1 )
        return _generate_ok ( id);
    else
        return _generate_fail (DB_ERROR_BADINPUT);
}

/**
 * \brief Updates in the table t_bios_client_info row by id.
 *
 * \param common_msg_t - message with new information
 *
 * \return COMMON_MSG_DB_FAIL if update failed
 *         COMMON_MSG_DB_OK   if update was successful
 */
common_msg_t* _update_client_info (const char* url, common_msg_t* newclientinfo)
{
    return _generate_fail (DB_ERROR_NOTIMPLEMENTED);  // TODO NOT INMPLEMENTED
}

/**
 * \brief Selects the last row the table t_bios_client_info row by id.
 *
 * \param common_msg_t - message with new information
 *
 * \return COMMON_MSG_DB_FAIL if update failed
 *         COMMON_MSG_DB_OK   if update was successful
 */
common_msg_t* select_client_info_last(const char* url, unsigned int client_id, unsigned int device_id)
{
    assert(client_id);  // is required
    assert(device_id);  // is required

    unsigned int id = 0;
    tntdb::Blob myBlob;
    tntdb::Datetime mydatetime;

    try{
        tntdb::Connection conn = tntdb::connectCached(url); 

        tntdb::Statement st = conn.prepareCached(
            " select "
            " v.id, v.datum, v.info"
            " from"
            " v_bios_client_info_last v"
            " where v.id_discovered_device = :id_devicediscovered and v.id_client = :id_client"
        );

        // Should return one row or nothing.
        tntdb::Row row = st.setInt("id_deviceidscovered", device_id).
                            setInt("id_client", client_id).
                            selectRow();

        row[0].get(id);
        assert(id);
    
        bool isNotNull = row[1].get(mydatetime);
        assert(isNotNull);
        

        isNotNull = row[2].get(myBlob);
        assert(isNotNull);
    }
    catch (const tntdb::NotFound &e) {
        return _generate_fail (DB_ERROR_NOTFOUND);
    }
    catch (const std::exception &e) {
        return _generate_fail (DB_ERROR_INTERNAL);
    }
    time_t mytime = utils::db::convertToCTime(mydatetime);
    return _generate_client_info(id, client_id, device_id, mytime, myBlob.data(), myBlob.size());
}

common_msg_t* _select_client_info(const char* url, unsigned int id)
{
    assert (id);

    unsigned int client_id = 0;
    unsigned int device_id = 0;
    tntdb::Datetime mydatetime;
    tntdb::Blob myBlob;

    try{
        tntdb::Connection conn = tntdb::connectCached(url); 

        tntdb::Statement st = conn.prepareCached(
            " select "
            " v.timestamp, v.ext, v.id_client , v.id_discovered_device"
            " from"
            " v_bios_client_info v"
            " where v.id = :id"
        );
        
        tntdb::Row row = st.setInt("id", id).selectRow();
          
        bool isNotNull = row[0].get(mydatetime);
        assert (isNotNull);
                               
        isNotNull = row[1].get(myBlob);
        assert(isNotNull);
    
        row[2].get(client_id);
        assert(client_id);
        
        row[3].get(device_id);
        assert(device_id);
    }
    catch (const tntdb::NotFound &e) {
        return _generate_fail (DB_ERROR_NOTFOUND);
    }
    catch (const std::exception &e) {
        return _generate_fail (DB_ERROR_INTERNAL);
    }
    time_t mytime = utils::db::convertToCTime(mydatetime);
    return _generate_client_info(id, client_id, device_id, mytime, myBlob.data(), myBlob.size());
}
