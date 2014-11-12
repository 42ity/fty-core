#include <czmq.h>
#include <tntdb/connect.h>
#include <tntdb/row.h>
#include <tntdb/error.h>
#include <tntdb/value.h>
#include <tntdb/result.h>

#include "log.h"
#include "assetmsg.h"
#include "common_msg.h"

/**
 * \brief Generates a COMMON_MSG_FAIL message.
 * 
 * Generates a COMMON_MSG_FAIL with error type BIOS_ERROR_DB , with a specified code, 
 * errormessage and optional parameters.
 * If errmsg is NULL, then it would be "".
 *
 * \param errorid - an id of the error.
 * \param errmsg  - a detailed message about the error.
 * \param erraux  - optional information.
 *
 * \return a common_msg_t message of the type COMMON_MSG_FAIL.
 *
 * TODO the codes are now defined as define. May be need to have enum
 */
common_msg_t* _generate_db_fail(uint32_t errorid, const char* errmsg, zhash_t** erraux)
{
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_FAIL);
    assert ( resultmsg );
    common_msg_set_errtype (resultmsg, BIOS_ERROR_DB);
    common_msg_set_errorno (resultmsg, errorid);
    common_msg_set_errmsg  (resultmsg, errmsg);
    if ( erraux != NULL )
    {
        common_msg_set_erraux  (resultmsg, erraux);
        zhash_destroy (erraux);
    }
    return resultmsg;
}

/**
 * \brief Generates an COMMON_MSG_DB_OK message and specifies an id of row that was processed.
 *
 * \param id - an id of the processed row.
 *
 * \return a common_msg_t message of the type COMMON_MSG_DB_OK.
 */
common_msg_t* _generate_ok(uint32_t rowid)
{
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_DB_OK);
    assert ( resultmsg );
    common_msg_set_rowid (resultmsg, rowid);
    return resultmsg;
}

////////////////////////////////////////////////////////////////////
///////            CLIENT                    ///////////////////////
////////////////////////////////////////////////////////////////////

common_msg_t* _generate_client(const char* name)
{
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_CLIENT);
    assert ( resultmsg );
    common_msg_set_name (resultmsg, name);
    return resultmsg;
}

//it shoud destroy the client
common_msg_t* _generate_return_client(uint32_t clientid, common_msg_t** client)
{
    assert ( client );
    assert ( *client );
    assert ( common_msg_id (*client) == COMMON_MSG_CLIENT );
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_RETURN_CLIENT);
    assert ( resultmsg );
    common_msg_set_rowid (resultmsg, clientid);
    zmsg_t* nnmsg = common_msg_encode (client);
    assert ( nnmsg );
    common_msg_set_msg (resultmsg, &nnmsg);
    return resultmsg;
}

common_msg_t* select_client(const char* url, const char* name)
{
    assert ( strlen(name) > 0 );

    int id_client = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            " v.id"
            " FROM"
            " v_bios_client v"
            " WHERE v.name = :name"
        );
          
        tntdb::Value val = st.setString("name", name).selectValue();
        val.get(id_client); 
    }
    catch (const tntdb::NotFound &e){
        return _generate_db_fail (DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        return _generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_t* client = _generate_client (name);
    return _generate_return_client (id_client, &client);
}

common_msg_t* select_client(const char* url, uint32_t id)
{
    std::string name = "";
    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            " v.name"
            " FROM"
            " v_bios_client v"
            " WHERE v.id = :id"
        );
          
        tntdb::Value val = st.setInt("id", id).selectValue();
        val.get(name);
        assert ( name != "" );
    }
    catch (const tntdb::NotFound &e){
        return _generate_db_fail(DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        return _generate_db_fail(DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_t* client = _generate_client (name.c_str());
    return _generate_return_client (id, &client);
}

common_msg_t* insert_client(const char* url,const char* name)
{
    uint32_t length = strlen(name);
    if ( ( length == 0 ) || ( length > MAX_NAME_LENGTH ) )
        return _generate_db_fail (DB_ERROR_BADINPUT, "length not in range [1,25]", NULL);

    uint32_t n = 0;
    uint32_t newid = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            " v_bios_client (id,name)"
            " VALUES (NULL,:name)"
        );
    
        n  = st.setString("name", name).execute();
        newid = conn.lastInsertId();
    }
    catch (const std::exception &e) {
        return _generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
        return _generate_ok (newid);
    else
        return _generate_db_fail (DB_ERROR_BADINPUT, NULL, NULL);
}

common_msg_t* delete_client(const char* url, uint32_t id_client)
{
    uint32_t n = 0;
    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            " v_bios_client "
            " WHERE id = :id"
        );
    
        n  = st.setInt("id", id_client).execute();
    } 
    catch (const std::exception &e) {
        return _generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
        return _generate_ok (id_client);
    else
        return _generate_db_fail (DB_ERROR_BADINPUT, NULL, NULL);
}
// should destroy client
common_msg_t* update_client(const char* url, uint32_t id, common_msg_t** client)
{
    assert ( client );
    assert ( *client );

    assert ( common_msg_id (*client) == COMMON_MSG_CLIENT );
    const char* name = common_msg_name (*client);
    uint32_t length = strlen(name);
    if ( ( length == 0 ) || ( length > MAX_NAME_LENGTH ) )
    {
        common_msg_destroy(client);
        return _generate_db_fail (DB_ERROR_BADINPUT, "length not in range [1,25]", NULL);
    }

    uint32_t n = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " UPDATE"
            " v_bios_client"
            " SET name = :name"
            " WHERE id = :id"
        );
    
        n  = st.setString("name", name).
                setInt("id", id).
                execute();
    }
    catch (const std::exception &e) {
        common_msg_destroy (client);
        return _generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_destroy (client);
    if ( n == 1 )
        return _generate_ok (id);
    else
        return _generate_db_fail (DB_ERROR_BADINPUT, NULL, NULL);
}

////////////////////////////////////////////////////////////////////////
/////////////////           CLIENT INFO              ///////////////////
////////////////////////////////////////////////////////////////////////

// Date is always UTC time as UNIX_timestamp.
//

common_msg_t* _generate_client_info
    (uint32_t client_id, uint32_t device_id, uint32_t mytime, byte* data, uint32_t datasize)
{
    assert ( datasize );
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_CLIENT_INFO);
    assert ( resultmsg );
    common_msg_set_client_id (resultmsg, client_id);
    common_msg_set_device_id (resultmsg, device_id);
    zchunk_t* blob = zchunk_new (data, datasize);
    common_msg_set_info (resultmsg, &blob);
    common_msg_set_date (resultmsg, mytime);
    return resultmsg;
}

//it shoud destroy the client_info.
common_msg_t* _generate_return_client_info(uint32_t client_info_id, common_msg_t** client_info)
{
    assert ( client_info );
    assert ( *client_info );

    assert ( common_msg_id (*client_info) == COMMON_MSG_CLIENT_INFO );
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_RETURN_CINFO);
    assert ( resultmsg );
    common_msg_set_rowid (resultmsg, client_info_id);
    zmsg_t* nnmsg = common_msg_encode (client_info);
    assert ( nnmsg );
    common_msg_set_msg (resultmsg, &nnmsg);

    return resultmsg;
}

/**
 * \brief Inserts into the table t_bios_client_info new row.
 *
 * blob would be destroyed.
 *
 * \param device_id - id of the device information is about
 * \param client_id - id of the module that gathered this information
 * \param blob      - an information as a flow of bytes
 *
 * \return COMMON_MSG_FAIL if inserting failed
 *         COMMON_MSG_DB_OK   if inserting was successful
 */
common_msg_t* insert_client_info
    (const char* url, uint32_t device_id, uint32_t client_id, zchunk_t** blob)
{
    assert ( device_id );  // is required
    assert ( client_id );  // is required
    assert ( blob );
    assert ( *blob );      // is required

    uint32_t n = 0;     // number of rows affected
    uint32_t newid = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            " v_bios_client_info (id, id_client, id_discovered_device, ext, timestamp)"
            " VALUES (NULL, :idclient, :iddiscovereddevice, :ext, UTC_TIMESTAMP())"
        );          // time is the time of inserting into database
        tntdb::Blob blobData((const char*)zchunk_data(*blob), zchunk_size(*blob));

        n = st.setInt("idclient", client_id).
               setInt("iddiscovereddevice", device_id).
               setBlob("ext", blobData).
               execute();
        newid = conn.lastInsertId();
    }
    catch (const std::exception &e) {
        zchunk_destroy (blob);
        return _generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    zchunk_destroy (blob);
    if ( n == 1 )
        return _generate_ok (newid);
    else
        return _generate_db_fail (DB_ERROR_BADINPUT, NULL, NULL);
}

common_msg_t* insert_client_info
    (const char* url, uint32_t device_id, uint32_t client_id, byte* blobdata, uint32_t blobsize)
{
    assert ( device_id );         // is required
    assert ( client_id );         // is required
    assert ( blobsize > 0 );      // is required

    zchunk_t* blob = zchunk_new(blobdata,blobsize);

    return insert_client_info(url,device_id, client_id, &blob);
}

/**
 * \brief Delets from the table t_bios_client_info row by id.
 *
 * \param id - id of the row to be deleted.
 *
 * \return COMMON_MSG_FAIL    if delete failed.
 *         COMMON_MSG_DB_OK   if delete was successful.
 */
common_msg_t* delete_client_info (const char* url, uint32_t id)
{
   uint32_t n = 0;
   try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            " v_bios_client_info "
            " WHERE id = :id"
        );
    
        n  = st.setInt("id", id).execute();
    }
    catch (const std::exception &e) {
        return _generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
        return _generate_ok (id);
    else
        return _generate_db_fail (DB_ERROR_BADINPUT, NULL, NULL);
}

/**
 * \brief Updates in the table t_bios_client_info row by id.
 *
 * \param common_msg_t - message with new information.
 *
 * \return COMMON_MSG_FAIL    if update failed.
 *         COMMON_MSG_DB_OK   if update was successful.
 */
common_msg_t* update_client_info (const char* url, common_msg_t** newclientinfo)
{
    return _generate_db_fail (DB_ERROR_NOTIMPLEMENTED, NULL, NULL);  // TODO NOT INMPLEMENTED
}

/**
 * \brief Selects the last row the table t_bios_client_info row by id.
 *
 * \param common_msg_t - message with new information.
 *
 * \return COMMON_MSG_FAIL    if update failed.
 *         COMMON_MSG_DB_OK   if update was successful.
 */
common_msg_t* select_client_info_last(const char* url, uint32_t client_id, uint32_t device_id)
{
    assert(client_id);  // is required
    assert(device_id);  // is required

    uint32_t id = 0;
    tntdb::Blob myBlob;
    time_t mydatetime = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url); 

        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            " v.id, UNIX_TIMESTAMP(v.datum) as utm, v.info"
            " FROM"
            " v_bios_client_info_last v"
            " WHERE v.id_discovered_device = :id_devicediscovered AND v.id_client = :id_client"
        );

        // Should return one row or nothing.
        tntdb::Row row = st.setInt("id_deviceidscovered", device_id).
                            setInt("id_client", client_id).
                            selectRow();

        row[0].get(id);
        assert ( id );
    
        bool isNotNull = row[1].get(mydatetime);
        assert ( isNotNull );

        isNotNull = row[2].get(myBlob);
        assert ( isNotNull );
    }
    catch (const tntdb::NotFound &e) {
        return _generate_db_fail (DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        return _generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    return _generate_client_info(client_id, device_id, mydatetime, (byte*)myBlob.data(), myBlob.size());
}

common_msg_t* select_client_info(const char* url, uint32_t id_client_info)
{
    assert ( id_client_info );

    uint32_t client_id = 0;
    uint32_t device_id = 0;
    time_t mydatetime = 0;
    tntdb::Blob myBlob;

    try{
        tntdb::Connection conn = tntdb::connectCached(url); 

        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            " UNIX_TIMESTAMP(v.timestamp) as utm, v.ext, v.id_client , v.id_discovered_device"
            " FROM"
            " v_bios_client_info v"
            " WHERE v.id = :id"
        );
        
        tntdb::Row row = st.setInt("id", id_client_info).selectRow();
          
        bool isNotNull = row[0].get(mydatetime);
        assert ( isNotNull );
                               
        isNotNull = row[1].get(myBlob);
        assert ( isNotNull );
    
        row[2].get(client_id);
        assert ( client_id );
        
        row[3].get(device_id);
        assert ( device_id );
    }
    catch (const tntdb::NotFound &e) {
        return _generate_db_fail (DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        return _generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_t* client_info = _generate_client_info(client_id, device_id, mydatetime, (byte*)myBlob.data(), myBlob.size());
    return _generate_return_client_info (id_client_info, &client_info);
}


///////////////////////////////////////////////////////////////////
///////            DEVICE TYPE              ///////////////////////
///////////////////////////////////////////////////////////////////

common_msg_t* _generate_device_type(const char* name)
{
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_DEVICE_TYPE);
    assert ( resultmsg );
    common_msg_set_name (resultmsg, name);
    return resultmsg;
}
//it should destroy the device type
common_msg_t* _generate_return_device_type(uint32_t devicetype_id, common_msg_t** device_type)
{

    assert ( device_type );
    assert ( *device_type );
    assert ( common_msg_id (*device_type) == COMMON_MSG_DEVICE_TYPE );
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_RETURN_DEVTYPE);
    assert ( resultmsg );
    common_msg_set_rowid (resultmsg, devicetype_id);
    zmsg_t* nnmsg = common_msg_encode (device_type);
    assert ( nnmsg );
    common_msg_set_msg (resultmsg, &nnmsg);
    return resultmsg;
}

common_msg_t* select_device_type(const char* url, const char* name)
{
    assert ( strlen(name) > 0 );

    int devicetype_id = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            " v.id"
            " FROM"
            " v_bios_device_type v"
            " WHERE v.name = :name"
        );
          
        tntdb::Value val = st.setString("name", name).selectValue();
        val.get(devicetype_id); 
    }
    catch (const tntdb::NotFound &e){
        return _generate_db_fail (DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        return _generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_t* devicetype = _generate_device_type (name);
    assert(devicetype);
    return _generate_return_device_type (devicetype_id, &devicetype);
}

common_msg_t* select_device_type(const char* url, uint32_t devicetype_id)
{
    std::string name = "";
    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            " v.name"
            " FROM"
            " v_bios_device_type v"
            " WHERE v.id = :id"
        );
          
        tntdb::Value val = st.setInt("id", devicetype_id).selectValue();
        val.get(name);
        assert ( name != "" );
    }
    catch (const tntdb::NotFound &e){
        return _generate_db_fail(DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        return _generate_db_fail(DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_t* devicetype = _generate_device_type (name.c_str());
    return _generate_return_device_type (devicetype_id, &devicetype);
}

common_msg_t* insert_device_type(const char* url, const char* name)
{
    uint32_t length = strlen(name);
    if ( ( length == 0 ) || ( length > MAX_NAME_LENGTH ) )
        return _generate_db_fail (DB_ERROR_BADINPUT, "length not in range [1,25]", NULL);

    uint32_t n = 0;
    uint32_t newid = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            " v_bios_device_type (id,name)"
            " VALUES (NULL,:name)"
        );
    
        n  = st.setString("name", name).execute();
        newid = conn.lastInsertId();
    }
    catch (const std::exception &e) {
        return _generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
        return _generate_ok (newid);
    else
        return _generate_db_fail (DB_ERROR_BADINPUT, NULL, NULL);
}

common_msg_t* delete_device_type(const char* url, uint32_t devicetype_id)
{
    uint32_t n = 0;
    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            " v_bios_device_type "
            " WHERE id = :id"
        );
    
        n  = st.setInt("id", devicetype_id).execute();
    } 
    catch (const std::exception &e) {
        return _generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
        return _generate_ok (devicetype_id);
    else
        return _generate_db_fail (DB_ERROR_BADINPUT, NULL, NULL);
}

// should destroy device_type
common_msg_t* update_device_type(const char* url, uint32_t id, common_msg_t** devicetype)
{
    assert ( common_msg_id (*devicetype) == COMMON_MSG_DEVICE_TYPE );
    const char* name = common_msg_name (*devicetype);
    uint32_t length = strlen(name);
    if ( ( length == 0 ) || ( length > MAX_NAME_LENGTH ) )
    {
        common_msg_destroy (devicetype);
        return _generate_db_fail (DB_ERROR_BADINPUT, "length not in range [1,25]", NULL);
    }

    uint32_t n = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " UPDATE"
            " v_bios_device_type"
            " SET name = :name"
            " WHERE id = :id"
        );
    
        n  = st.setString("name", name).
                setInt("id",id).
                execute();
    }
    catch (const std::exception &e) {
        common_msg_destroy (devicetype);
        return _generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_destroy(devicetype);
    if ( n == 1 )
        return _generate_ok (id);
    else
        return _generate_db_fail (DB_ERROR_BADINPUT, NULL, NULL);
}
////////////////////////////////////////////////////////////////////////
/////////////////           DEVICE                   ///////////////////
////////////////////////////////////////////////////////////////////////

common_msg_t* _generate_device(const char* name, uint32_t devicetype_id)
{
    assert ( devicetype_id );
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_DEVICE);
    assert ( resultmsg );
    common_msg_set_name (resultmsg, name);
    common_msg_set_devicetype_id (resultmsg, devicetype_id);

    return resultmsg;
}

//it should destroy the device
common_msg_t* _generate_return_device(uint32_t device_id, common_msg_t** device)
{
    assert ( common_msg_id (*device) == COMMON_MSG_DEVICE );
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_RETURN_DEVICE);
    assert ( resultmsg );
    common_msg_set_rowid (resultmsg, device_id);
    zmsg_t* nnmsg = common_msg_encode (device);
    assert ( nnmsg );
    common_msg_set_msg (resultmsg, &nnmsg);

    return resultmsg;
}

/**
 * \brief Inserts into the table t_bios_discovered_device new row.
 *
 * \param url           - connection url to database.
 * \param devicetype_id - id of the device type.
 * \param name          - name of the device.
 *
 * \return COMMON_MSG_FAIL    if inserting failed.
 *         COMMON_MSG_DB_OK   if inserting was successful.
 */
common_msg_t* insert_device(const char* url, uint32_t devicetype_id, const char* name)
{
    assert ( devicetype_id ); // is required
    assert ( strlen(name) );  // is required

    uint32_t n = 0;     // number of rows affected.
    uint32_t newid = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            " v_bios_discovered_device (id, name, id_device_type)"
            " VALUES (NULL, :name, :iddevicetype)"
        );
    
        // Insert one row or nothing
        n  = st.setString("name", name).
                setInt("iddevicetype", devicetype_id).
                execute();

        newid = conn.lastInsertId();
    }
    catch (const std::exception &e) {
        return _generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
        return _generate_ok (newid);
    else
        return _generate_db_fail (DB_ERROR_BADINPUT, NULL, NULL);
}

/**
 * \brief Deletes from the table t_bios_discovered_device row by id.
 *
 * \param id - id of the row to be deleted.
 *
 * \return COMMON_MSG_FAIL    if delete failed.
 *         COMMON_MSG_DB_OK   if delete was successful.
 */
common_msg_t* delete_device (const char* url, uint32_t device_id)
{
   uint32_t n = 0;
   try{
        tntdb::Connection conn = tntdb::connectCached(url);
   
        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            " v_bios_discovered_device "
            " WHERE id = :id"
        );

        n  = st.setInt("id", device_id).execute();
    }
    catch (const std::exception &e) {
        return _generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
        return _generate_ok (device_id);
    else
        return _generate_db_fail (DB_ERROR_BADINPUT, NULL, NULL);
}

/**
 * \brief Updates in the table t_bios_discovered_device row by id.
 *
 * \param newdevice - message with new information.
 *
 * \return COMMON_MSG_FAIL if update failed.
 *         COMMON_MSG_DB_OK   if update was successful.
 */
common_msg_t* update_device (const char* url, common_msg_t** newdevice)
{
    return _generate_db_fail (DB_ERROR_NOTIMPLEMENTED, NULL, NULL);  // TODO NOT INMPLEMENTED
}

common_msg_t* select_device (const char* url, uint32_t devicetype_id, const char* name)
{
    assert ( devicetype_id );
    assert ( strlen(name) > 0 );

    try{
        tntdb::Connection conn = tntdb::connectCached(url); 

        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            " v.id"
            " FROM"
            " v_bios_discovered_device v"
            " WHERE v.name = :name AND v.id_device_type = :devicetypeid"
        );
        
        tntdb::Result result = st.setInt("devicetypeid", devicetype_id).
                                  setString("name",name).
                                  select();
        
        uint32_t rsize = result.size();
        uint32_t id = 0;

        if ( rsize > 0 )
        {
            // There are discovered devices with such name
            // TODO for now take first in the list
            auto row= *(result.begin()); 

            row[0].get(id);

            common_msg_t* device = _generate_device (name, devicetype_id);
            return _generate_return_device (id, &device);
        }
        else
            return _generate_db_fail (DB_ERROR_NOTFOUND, NULL, NULL);
    }
    catch (const std::exception &e) {
        return _generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
}

common_msg_t* select_device (const char* url, const char* devicetype_name, const char* name)
{
    assert ( strlen(devicetype_name) > 0 );
    assert ( strlen(name) > 0 );

    // find devicetype_id
    common_msg_t* device_type = select_device_type(url, devicetype_name);
    assert ( device_type );
    uint32_t msgid = common_msg_id (device_type);
    uint32_t rowid = common_msg_rowid (device_type);
    common_msg_destroy (&device_type);

    if ( msgid == COMMON_MSG_FAIL )
        return _generate_db_fail (DB_ERROR_BADINPUT, "unknown device type", NULL);
    else if ( msgid == COMMON_MSG_RETURN_DEVTYPE )
        return select_device(url, rowid, name);
    else
        assert (false); // unknown return type
    return NULL;
}

common_msg_t* insert_device(const char* url, const char* devicetype_name, const char* name)
{
    assert ( strlen(devicetype_name) > 0 );
    assert ( strlen(name) > 0 );

    // find devicetype_id
    common_msg_t* device_type = select_device_type(url, devicetype_name);
    assert ( device_type );
    uint32_t msgid = common_msg_id (device_type);
    uint32_t rowid = common_msg_rowid (device_type);
    common_msg_destroy (&device_type);

    if ( msgid == COMMON_MSG_FAIL )
        return _generate_db_fail (DB_ERROR_BADINPUT, "unknown device type", NULL);
    else if ( msgid == COMMON_MSG_RETURN_DEVTYPE )
        return insert_device(url, rowid , name);
    else
        assert (false); // unknown return type
    return NULL;
}

common_msg_t* _generate_key(const char* keytagname, uint32_t scale)
{
    assert ( scale );
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_KEY);
    assert ( resultmsg );
    common_msg_set_keytagname (resultmsg, keytagname);
    common_msg_set_scale (resultmsg, scale);

    return resultmsg;
}

//it should destroy the key
common_msg_t* _generate_return_key(uint32_t keytag_id, common_msg_t** key)
{
    assert ( common_msg_id (*key) == COMMON_MSG_KEY );
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_RETURN_KEY);
    assert ( resultmsg );
    common_msg_set_rowid (resultmsg, keytag_id);
    zmsg_t* nnmsg = common_msg_encode (key);
    assert ( nnmsg );
    common_msg_set_msg (resultmsg, &nnmsg);

    return resultmsg;
}

common_msg_t* select_key (const char* url, const char* keytagname)
{
    assert ( strlen(keytagname) > 0 );

    try{
        tntdb::Connection conn = tntdb::connectCached(url); 

        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            " v.id , v.scale"
            " FROM"
            " v_bios_measurements v"
            " WHERE v.keytag = :name"
        );
        
        tntdb::Row row = st.setString("name", keytagname).
                            selectRow();
        
        uint32_t rowid = 0;
        row[0].get(rowid);

        uint32_t scale = 0;
        row[1].get(scale);      // HOW to work with double? or it would be integer too?

        common_msg_t* key = _generate_key (keytagname, scale);
        return _generate_return_key (rowid, &key);
    }
    catch (const tntdb::NotFound &e){
        return _generate_db_fail (DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        return _generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    return NULL;
}

/**
 * \brief Inserts into the table t_bios_client_measurements new row.
 *
 * \param url          - connection url to database.
 * \param client_id    - id of the client that get measure.
 * \param device_id    - id of the device that was measured.
 * \param keytag_id    - id of the keytag, that indicates the measurement
 * \param subkeytag_id - id of the subkeytag, number that indikates the hierarchie
 * \param value        - value of measurement
 *
 * \return COMMON_MSG_FAIL    if inserting failed.
 *         COMMON_MSG_DB_OK   if inserting was successful.
 */
common_msg_t* insert_measurement(const char* url, uint32_t client_id, 
                                 uint32_t device_id, uint32_t keytag_id, 
                                 uint32_t subkeytag_id, uint64_t value)
{
    assert ( client_id );    // is required
    assert ( device_id );    // is required (if not device was measured, then use "dummy_monitor_device")
    assert ( keytag_id );    // is required
    assert ( subkeytag_id ); // is required

    uint32_t n = 0;     // number of rows affected.
    uint32_t newid = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            " v_bios_client_info_measurements (id, id_client, id_device, id_key , id_subkey, value, timestamp)"
            " VALUES (NULL, :clientid, :deviceid, :keytagid, :subkeytagid , :val, UTC_TIMESTAMP())"
        );
    
        // Insert one row or nothing
        n  = st.setInt("clientid", client_id).
                setInt("deviceid", device_id).
                setInt("keytagid", keytag_id).
                setInt("subkeytagid", subkeytag_id).
                setInt("val", value).
                execute();

        newid = conn.lastInsertId();
    }
    catch (const std::exception &e) {
        return _generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
        return _generate_ok (newid);
    else
        return _generate_db_fail (DB_ERROR_BADINPUT, NULL, NULL);
}


