#include <czmq.h>
#include <tntdb/connect.h>
#include <tntdb/row.h>
#include <tntdb/error.h>
#include <tntdb/value.h>
#include <tntdb/result.h>

#include "common_msg.h"
#include "log.h"
#include "dbpath.h"
#include "defs.h"
#include "persist_error.h"
#include "assetmsg.h"

// Date is always UTC time as UNIX_timestamp.
/**
 * \brief Generates a COMMON_MSG_FAIL message.
 * 
 * Generates a COMMON_MSG_FAIL with error type BIOS_ERROR_DB , with 
 * a specified code, errormessage and optional parameters.
 * 
 * If the parameter errmsg was NULL, then result error message would be "".
 *
 * \param errorid - an id of the error.
 * \param errmsg  - a detailed message about the error.
 * \param erraux  - optional information.
 *
 * \return a common_msg_t message of the type COMMON_MSG_FAIL.
 *
 * TODO the codes are now defined as define. May be need to have enum
 */
common_msg_t* generate_db_fail(uint32_t errorid, const char* errmsg, 
                               zhash_t** erraux)
{
    log_info ("%s \n", "start");
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_FAIL);
    assert ( resultmsg );
    common_msg_set_errtype (resultmsg, BIOS_ERROR_DB);
    common_msg_set_errorno (resultmsg, errorid);
    common_msg_set_errmsg  (resultmsg, "%s", (errmsg ? errmsg:"") );
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
common_msg_t* generate_ok(uint32_t rowid)
{
    log_info ("%s \n", "start");
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_DB_OK);
    assert ( resultmsg );
    common_msg_set_rowid (resultmsg, rowid);
    return resultmsg;
}

////////////////////////////////////////////////////////////////////
///////            CLIENT                    ///////////////////////
////////////////////////////////////////////////////////////////////

common_msg_t* generate_client(const char* name)
{
    log_info ("%s \n", "start");
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_CLIENT);
    assert ( resultmsg );
    common_msg_set_name (resultmsg, name);
    return resultmsg;
}

//it shoud destroy the client
common_msg_t* generate_return_client(uint32_t clientid, common_msg_t** client)
{
    log_info ("%s \n", "start");
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
    log_info ("%s \n", "start");
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
            " LIMIT 1"
        );
          
        tntdb::Value val = st.setString("name", name).selectValue();
        val.get(id_client); 
    }
    catch (const tntdb::NotFound &e){
        return generate_db_fail (DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_t* client = generate_client (name);
    return generate_return_client (id_client, &client);
}

common_msg_t* select_client(const char* url, uint32_t id)
{
    log_info ("%s \n", "start");
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
        assert ( !name.empty() );
    }
    catch (const tntdb::NotFound &e){
        return generate_db_fail(DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        return generate_db_fail(DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_t* client = generate_client (name.c_str());
    return generate_return_client (id, &client);
}

common_msg_t* insert_client(const char* url,const char* name)
{
    log_info ("%s \n", "start");
    uint32_t length = strlen(name);
    if ( ( length == 0 ) || ( length > MAX_NAME_LENGTH ) )
        return generate_db_fail (DB_ERROR_BADINPUT, "length not in range [1,25]", NULL);

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
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
        return generate_ok (newid);
    else
        return generate_db_fail (DB_ERROR_BADINPUT, NULL, NULL);
}

common_msg_t* delete_client(const char* url, uint32_t id_client)
{
    log_info ("%s \n", "start");
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
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
        return generate_ok (id_client);
    else
        return generate_db_fail (DB_ERROR_BADINPUT, NULL, NULL);
}
// should destroy client
common_msg_t* update_client(const char* url, uint32_t id, common_msg_t** client)
{
    log_info ("%s \n", "start");
    assert ( client );
    assert ( *client );

    assert ( common_msg_id (*client) == COMMON_MSG_CLIENT );
    const char* name = common_msg_name (*client);
    uint32_t length = strlen(name);
    if ( ( length == 0 ) || ( length > MAX_NAME_LENGTH ) )
    {
        common_msg_destroy(client);
        return generate_db_fail (DB_ERROR_BADINPUT, "length not in range [1,25]", NULL);
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
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_destroy (client);
    if ( n == 1 )
        return generate_ok (id);
    else
        return generate_db_fail (DB_ERROR_BADINPUT, NULL, NULL);
}

////////////////////////////////////////////////////////////////////////
/////////////////           CLIENT INFO              ///////////////////
////////////////////////////////////////////////////////////////////////

// Date is always UTC time as UNIX_timestamp.
//

common_msg_t* generate_client_info
    (uint32_t client_id, uint32_t device_id, uint32_t mytime, byte* data, uint32_t datasize)
{
    log_info ("%s \n", "start");
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
common_msg_t* generate_return_client_info(uint32_t client_info_id, common_msg_t** client_info)
{
    log_info ("%s \n", "start");
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
    log_info ("%s \n", "start");
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
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    zchunk_destroy (blob);
    if ( n == 1 )
        return generate_ok (newid);
    else
        return generate_db_fail (DB_ERROR_BADINPUT, NULL, NULL);
}

common_msg_t* insert_client_info
    (const char* url, uint32_t device_id, uint32_t client_id, byte* blobdata, uint32_t blobsize)
{
    log_info ("%s \n", "start");
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
    log_info ("%s \n", "start");
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
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
        return generate_ok (id);
    else
        return generate_db_fail (DB_ERROR_BADINPUT, NULL, NULL);
}

/**
 * \brief Updates into the table t_bios_client_info new row.
 *
 * blob would be destroyed.
 *
 * \param client_id - id of the module that gathered this information
 * \param blob      - an information as a flow of bytes
 *
 * \return COMMON_MSG_FAIL if update failed
 *         COMMON_MSG_DB_OK   if update was successful
 */
common_msg_t* update_client_info
    (const char* url, uint32_t client_id, zchunk_t** blob)
{
    log_info ("%s \n", "start");
    assert ( client_id );  // is required
    assert ( blob );
    assert ( *blob );      // is required

    uint32_t n = 0;     // number of rows affected
    uint32_t newid = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " UPDATE t_bios_client_info"
            " SET ext=:ext, timestamp=UTC_TIMESTAMP()"
            " WHERE id_client=:idclient"
        );          // time is the time of inserting into database
        tntdb::Blob blobData((const char*)zchunk_data(*blob), zchunk_size(*blob));

        n = st.setInt("idclient", client_id).
               setBlob("ext", blobData).
               execute();
        newid = conn.lastInsertId();
    }
    catch (const std::exception &e) {
        zchunk_destroy (blob);
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    zchunk_destroy (blob);
    if ( n == 1 )
        return generate_ok (newid);
    else
        return generate_db_fail (DB_ERROR_BADINPUT, NULL, NULL);
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
    log_info ("%s \n", "start");
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
        return generate_db_fail (DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    return generate_client_info(client_id, device_id, mydatetime, (byte*)myBlob.data(), myBlob.size());
}

common_msg_t* select_client_info(const char* url, uint32_t id_client_info)
{
    log_info ("%s \n", "start");
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
        return generate_db_fail (DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_t* client_info = generate_client_info(client_id, device_id, mydatetime, (byte*)myBlob.data(), myBlob.size());
    return generate_return_client_info (id_client_info, &client_info);
}

// specific function due to unusual nature of ui/properties entry - we suppose there will be
// the only one
common_msg_t* select_ui_properties(const char* url)
{
    log_info ("%s \n", "start");
    uint32_t id_client_info = 0;
    uint32_t device_id = 0;
    time_t mydatetime = 0;
    tntdb::Blob myBlob;

    try{
        tntdb::Connection conn = tntdb::connectCached(url); 

        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            " UNIX_TIMESTAMP(v.timestamp) as utm, v.ext, v.id , v.id_discovered_device"
            " FROM"
            " v_bios_client_info v"
            " WHERE v.id_client = :id"
        );
        
        tntdb::Row row = st.setInt("id", UI_PROPERTIES_CLIENT_ID).selectRow();
          
        bool isNotNull = row[0].get(mydatetime);
        assert ( isNotNull );
                               
        isNotNull = row[1].get(myBlob);
        assert ( isNotNull );
    
        row[2].get(id_client_info);
        assert ( id_client_info );
        
        row[3].get(device_id);
    }
    catch (const tntdb::NotFound &e) {
        return generate_db_fail (DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_t* client_info = generate_client_info(UI_PROPERTIES_CLIENT_ID, device_id, mydatetime, (byte*)myBlob.data(), myBlob.size());
    return generate_return_client_info (id_client_info, &client_info);
}



///////////////////////////////////////////////////////////////////
///////            DEVICE TYPE              ///////////////////////
///////////////////////////////////////////////////////////////////

common_msg_t* generate_device_type(const char* name)
{
    log_info ("%s \n", "start");
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_DEVICE_TYPE);
    assert ( resultmsg );
    common_msg_set_name (resultmsg, name);
    return resultmsg;
}
//it should destroy the device type
common_msg_t* generate_return_device_type(uint32_t devicetype_id, common_msg_t** device_type)
{

    log_info ("%s \n", "start");
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
    log_info ("%s \n", "start");
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
        return generate_db_fail (DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_t* devicetype = generate_device_type (name);
    assert(devicetype);
    return generate_return_device_type (devicetype_id, &devicetype);
}

common_msg_t* select_device_type(const char* url, uint32_t devicetype_id)
{
    log_info ("%s \n", "start");
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
        assert ( !name.empty() );
    }
    catch (const tntdb::NotFound &e){
        return generate_db_fail(DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        return generate_db_fail(DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_t* devicetype = generate_device_type (name.c_str());
    return generate_return_device_type (devicetype_id, &devicetype);
}

common_msg_t* insert_device_type(const char* url, const char* name)
{
    log_info ("%s \n", "start");
    uint32_t length = strlen(name);
    if ( ( length == 0 ) || ( length > MAX_NAME_LENGTH ) )
        return generate_db_fail (DB_ERROR_BADINPUT, 
                                "length not in range [1,25]", NULL);

    uint32_t n = 0;
    uint32_t newid = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            " v_bios_device_type (id,name)"
            " select NULL,:name from dual where not exists"
            " (select id from v_bios_device_type where name=:name)"
        );
    
        // TODO set
        n  = st.set("name", name).
                execute();
        newid = conn.lastInsertId();
    }
    catch (const std::exception &e) {
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
        return generate_ok (newid);
    else
        return generate_db_fail (DB_ERROR_BADINPUT, NULL, NULL);
}

common_msg_t* delete_device_type(const char* url, 
                                 m_dvc_tp_id_t device_type_id)
{
    log_info ("%s \n", "start");
    m_dvc_tp_id_t n = 0;
    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            " v_bios_device_type "
            " WHERE id = :id"
        );
    
        // TODO set
        n  = st.set("id", device_type_id).
                execute();
    } 
    catch (const std::exception &e) {
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
    {
        return generate_ok (device_type_id);
    }
    else
    {
        return generate_db_fail (DB_ERROR_BADINPUT, NULL, NULL);
    }
}

/**
 * \brief Checks if length is in the ragne [1,MAX_NAME_LENGTH]
 *
 * \param name - name to check
 *
 * \return true if name is in range
 *         false if name is not in range
 */
bool is_ok_name_length (const char* name)
{
    size_t length = strlen (name);
    if ( ( length == 0 ) || ( length > MAX_NAME_LENGTH ) )
        return false;
    else 
        return true;
}

/**
 * \brief Updates a row in a table t_bios_device_type by id with
 * specified object.
 *
 * In case of success it generates COMMON_MSG_OK message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * It destroyes the "device_type" message.
 *
 * \param url            - connection url to database.
 * \param device_type_id - id of the device type.
 * \param device_type    - a message COMMON_MSG_DEVICE_TYPE with new values.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_OK message.
 */
common_msg_t* update_device_type(const char* url, 
                                 m_dvc_tp_id_t device_type_id, 
                                 common_msg_t** device_type)
{
    log_info ("%s \n", "start");
    assert ( common_msg_id (*device_type) == COMMON_MSG_DEVICE_TYPE );
    const char* device_type_name = common_msg_name (*device_type);

    if ( !is_ok_name_length (device_type_name) )
    {
        common_msg_destroy (device_type);
        log_info ("too long device type name %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
        "device type name length not in range [1, MAX_NAME_LENGTH]", NULL);
    }

    m_dvc_tp_id_t n = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " UPDATE"
            " v_bios_device_type"
            " SET name = :name"
            " WHERE id = :id"
        );
        // TODO set
        n  = st.set("name", device_type_name).
                set("id", device_type_id).
                execute();
        log_debug ("was updated %d rows", n);
    }
    catch (const std::exception &e) {
        common_msg_destroy (device_type);
        log_warning ("abnormal %s \n", "end");
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_destroy (device_type);
    if ( n == 1 )
    {
        log_info ("normal %s \n", "end");
        return generate_ok (device_type_id);
    }
    else
    {   
        log_info ("nothing was updated %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
                                        "nothing was updated", NULL);
    }
}

////////////////////////////////////////////////////////////////////////
/////////////////           DEVICE                   ///////////////////
////////////////////////////////////////////////////////////////////////

/**
 * \brief A helper function analogue for common_msg_device_encode.
 *
 * But instead of zmsg_t returns common_msg_t.
 *
 * \param device_name    - name of the device.
 * \param device_type_id - an id of the device type.
 *
 * \return - a COMMON_MSG_DEVICE message.
 */
common_msg_t* generate_device (const char* device_name, 
                               m_dvc_tp_id_t device_type_id)
{
    log_info ("%s \n", "start");
    // TODO check name length according xml length
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_DEVICE);
    
    common_msg_set_name (resultmsg, device_name);
    common_msg_set_devicetype_id (resultmsg, device_type_id);

    log_info ("normal %s \n", "end");
    return resultmsg;
}

/**
 * \brief A helper function analogue for common_msg_return_device_encode.
 *
 * But instead of zmsg_t returns common_msg_t.
 *
 * It destroys "device" message.
 *
 * \param device_id - an id of the device in monitor part.
 * \param device    - a COMMON_MSG_DEVICE message.
 *
 * \return - a COMMON_MSG_RETURN_DEVICE message.
 */
common_msg_t* generate_return_device(m_dvc_id_t device_id, 
                                     common_msg_t** device)
{
    log_info ("%s \n", "start");
    assert ( common_msg_id (*device) == COMMON_MSG_DEVICE );
    
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_RETURN_DEVICE);
    
    zmsg_t* nnmsg = common_msg_encode (device);
    assert ( nnmsg );
    
    common_msg_set_msg (resultmsg, &nnmsg);
    common_msg_set_rowid (resultmsg, device_id);
    
    zmsg_destroy (&nnmsg);
    log_info ("normal %s \n", "end");
    return resultmsg;
}

/**
 * \brief Inserts into the table t_bios_discovered_device new row.
 *
 * In case of success it generates COMMON_MSG_OK message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url            - connection url to database.
 * \param device_type_id - id of the device type.
 * \param device_name    - name of the device.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_OK message.
 */
common_msg_t* insert_device(const char* url, m_dvc_tp_id_t device_type_id, 
                            const char* device_name)
{
    log_info ("%s \n", "start");
    assert ( device_type_id );       // is required
    
    if ( !is_ok_name_length (device_name) )
    {
        log_info ("too long device name %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
            "device name length not in range [1, MAX_NAME_LENGTH]", NULL);
    }

    m_dvc_id_t n     = 0;     // number of rows affected.
    m_dvc_id_t newid = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            " v_bios_discovered_device (id, name, id_device_type)"
            " VALUES (NULL, :name, :iddevicetype)"
        );
    
        // Insert one row or nothing
        // TODO set
        n  = st.set("name", device_name).
                set("iddevicetype", device_type_id).
                execute();
        // TODO use apropriate format string
        log_debug ("was %d rows inserted", n);

        newid = conn.lastInsertId();
    }
    catch (const std::exception &e) {
        log_warning ("abnormal %s \n", "end");
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
    {
        log_info ("normal %s \n", "end");
        return generate_ok (newid);
    }
    else
    {
        // TODO need to return existing ID????
        log_info ("nothing was inserted %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
                                                "noting was inserted", NULL);
    }
}

/**
 * \brief Deletes from the table t_bios_discovered_device row by id.
 *
 * In case of success it generates COMMON_MSG_OK message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url - connection url to database.
 * \param id  - id of the row to be deleted.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_OK message.
 */
common_msg_t* delete_device (const char* url, m_dvc_id_t device_id)
{
    log_info ("%s \n", "start");
    m_dvc_id_t n = 0;
    try{
        tntdb::Connection conn = tntdb::connectCached(url);
   
        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            " v_bios_discovered_device "
            " WHERE id = :id"
        );

        // TODO set
        n  = st.set("id", device_id).
                execute();
        log_debug ("was %d rows deleted", n);
    }
    catch (const std::exception &e) {
        log_warning ("abnormal %s \n", "end");
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
    {
        log_info ("normal %s \n", "end");
        return generate_ok (device_id);
    }
    else
    {
        log_info ("badinput %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
                                        "nothing was deleted", NULL);
    }
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
    return generate_db_fail (DB_ERROR_NOTIMPLEMENTED, NULL, NULL);  
    // TODO NOT INMPLEMENTED
}

/**
 * \brief Selects a device by device name in monitor part and device type id 
 * in monitor part.
 *
 * In case of success it generates COMMON_MSG_RETURN_DEVICE message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url            - connection url to database.
 * \param device_type_id - a device type name.
 * \param device_name    - a device name.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_RETURN_DEVICE message.
 */
common_msg_t* select_device (const char* url, m_dvc_tp_id_t device_type_id, 
                             const char* device_name)
{
    log_info ("%s \n", "start");
    assert ( device_type_id );
    
    if ( !is_ok_name_length (device_name) )
    {
        log_info ("too long device name %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
            "device name length not in range [1, MAX_NAME_LENGTH]", NULL);
    }

    try{
        tntdb::Connection conn = tntdb::connectCached(url); 

        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            " v.id"
            " FROM"
            " v_bios_discovered_device v"
            " WHERE v.name = :name AND v.id_device_type = :devicetypeid"
            " LIMIT 1"
        );
        
        // TODO set
        tntdb::Result result = st.set("devicetypeid", device_type_id).
                                  set("name", device_name).
                                  select();
        auto rsize = result.size();
        log_debug ("was %d rows selected", rsize);
        
        m_dvc_id_t id = 0;

        if ( rsize > 0 )
        {
            // There are discovered devices with such name
            // TODO for now take first in the list
            auto row= *(result.begin()); 

            row[0].get(id);
            assert ( id );  // database, was corrupted

            common_msg_t* device = generate_device (device_name, 
                                                    device_type_id);
            log_info ("normal %s \n", "end");
            return generate_return_device (id, &device);
        }
        else
        {   
            log_info ("notfound %s \n", "end");
            return generate_db_fail (DB_ERROR_NOTFOUND, 
                                            "no devices was found", NULL);
        }
    }
    catch (const std::exception &e) {
        log_warning ("abnormal %s \n", "end");
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
}

/**
 * \brief Selects a device by device name and device type name in 
 * monitor part.
 *
 * In case of success it generates COMMON_MSG_RETURN_DEVICE message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url              - connection url to database.
 * \param device_type_name - a device type name.
 * \param device_name      - a device name.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_RETURN_DEVICE message.
 */
common_msg_t* select_device (const char* url, const char* device_type_name, 
                             const char* device_name)
{
    log_info ("%s \n", "start");
 
    if ( !is_ok_name_length (device_name) )
    {
        log_info ("too long device name %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
            "device name length not in range [1, MAX_NAME_LENGTH]", NULL);
    }
    if ( !is_ok_name_length (device_type_name) )
    {
        log_info ("too long device type name %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
        "device type name length not in range [1, MAX_NAME_LENGTH]", NULL);
    }

    // find devicetype_id
    common_msg_t* device_type = select_device_type(url, device_type_name);
    assert ( device_type );
    uint32_t msgid = common_msg_id (device_type);

    switch (msgid){
        case COMMON_MSG_FAIL:
        {
            // send up
            log_info ("problems with device type identifying  %s \n", "end");
            return device_type;
        }
        case COMMON_MSG_RETURN_DEVTYPE:
        {   
            m_dvc_tp_id_t rowid = common_msg_rowid (device_type);
            common_msg_t *result = select_device(url, rowid, device_name);
            common_msg_destroy (&device_type);
            log_info ("normal %s \n", "end");
            return result;
        }
        default:
        {
            // this should never happen
            common_msg_destroy (&device_type);
            log_error ("this should never happen %s \n", "end");
            return generate_db_fail (DB_ERROR_INTERNAL,
                                        "unknown return type", NULL); 
        }
    }
}

/**
 * \brief Inserts a new device by device name and device type name into 
 * monitor part.
 *
 * In case of success it generates COMMON_MSG_OK message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url              - connection url to database.
 * \param device_type_name - a device type name.
 * \param device_name      - a device name.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_OK message.
 */
common_msg_t* insert_device(const char* url, const char* device_type_name, 
                            const char* device_name)
{
    log_info ("%s \n", "start");
    
    if ( !is_ok_name_length (device_name) )
    {
        log_info ("too long device name %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
            "device name length not in range [1, MAX_NAME_LENGTH]", NULL);
    }
    if ( !is_ok_name_length (device_type_name) )
    {
        log_info ("too long device type name %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
        "device type name length not in range [1, MAX_NAME_LENGTH]", NULL);
    }

    // find devicetype_id
    common_msg_t* device_type = select_device_type(url, device_type_name);
    assert ( device_type );
    uint32_t msgid = common_msg_id (device_type);

    switch (msgid){
        case COMMON_MSG_FAIL:
        {
            // send up
            log_info ("problems with device type identifying %s \n", "end");
            return device_type;
        }
        case COMMON_MSG_RETURN_DEVTYPE:
        {   
            m_dvc_tp_id_t rowid = common_msg_rowid (device_type);
            common_msg_t *result = insert_device(url, rowid, device_name);
            common_msg_destroy (&device_type);
            log_info ("normal %s \n", "end");
            return result;
        }
        default:
        {
            // this should never happen
            common_msg_destroy (&device_type);
            log_error ("this should never happen %s \n", "end");
            return generate_db_fail (DB_ERROR_INTERNAL,
                                        "unknown return type", NULL); 
        }
    }
}

////////////////////////////////////////////////////////////////////////
/////////////////           MEASUREMENT              ///////////////////
////////////////////////////////////////////////////////////////////////

/**
 * \brief Inserts into the table t_bios_client_measurements new row.
 *
 * In case of success it generates COMMON_MSG_OK message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 *
 * \param url        - connection url to database.
 * \param client_id  - id of the client that gathered measure.
 * \param device_id  - id of the device that was measured.
 * \param type_id    - id of the measure type.
 * \param subtype_id - id of the measure subtype.
 * \param value      - value of measurement.
 *
 * \return - a COMMON_MSG_FAIL or COMMON_MSG_DB_OK message.
 */
common_msg_t* insert_measurement(const char* url, m_clnt_id_t client_id, 
                                 m_dvc_id_t device_id, 
                                 m_msrmnt_tp_id_t type_id, 
                                 m_msrmnt_sbtp_id_t subtype_id, 
                                 m_msrmnt_value_t value)
{
    log_info ("%s \n", "start");
    assert ( client_id );    // is required
    assert ( device_id );    // is required (if not device was measured, 
                             // then use "dummy_monitor_device") 
                             // TODO insert script
    assert ( type_id );      // is required
    assert ( subtype_id );   // is required

    m_msrmnt_id_t n     = 0;     // number of rows affected.
    m_msrmnt_id_t newid = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            " v_bios_client_info_measurements"
            "   (id, id_client, id_discovered_device, id_key,"
            "    id_subkey, value, timestamp)"
            " VALUES"
            "   (NULL, :clientid, :deviceid, :keytagid, :subkeytagid,"
            "   :val, UTC_TIMESTAMP())"
        );
    
        // Insert one row or nothing
        // TODO set
        n  = st.set("clientid", client_id).
                set("deviceid", device_id).
                set("keytagid", type_id).
                set("subkeytagid", subtype_id).
                set("val", value).
                execute();
        // TODO use apropriate format strings
        log_info ("was inserted %ld rows\n", n);

        newid = conn.lastInsertId();
    }
    catch (const std::exception &e) {
        log_warning ("abnormal %s \n", "end");
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
    {
        log_info ("normal %s \n", "end");
        return generate_ok (newid);
    }
    else
    {
        // TODO need to return existing ID????
        log_info ("nothing was inserted %s \n", "end");
        return generate_db_fail (DB_ERROR_NOTHINGINSERTED, 
                                                "nothing was inserted", NULL);
    }
}

/**
 * \brief A helper function analogue for 
 *                              common_msg_return_last_measurements_encode.
 *
 * But instead of zmsg_t returns common_msg_t.
 *
 * It destroys "measurements".
 *
 * \param device_id    - an id_asset_element for the device.
 * \param measurements - last measurements for the specified device.
 *
 * \return - a COMMON_MSG_RETURN_LAST_MEASUREMENTS message.
 */
common_msg_t* generate_return_last_measurements (a_elmnt_id_t device_id, 
                                                zlist_t** measurements)
{
    log_info ("%s \n", "start");

    common_msg_t* resultmsg = common_msg_new 
                                    (COMMON_MSG_RETURN_LAST_MEASUREMENTS);
    common_msg_set_device_id (resultmsg, device_id);
    common_msg_set_measurements (resultmsg, measurements);
    
    zlist_destroy (measurements);
    log_info ("normal %s \n", "end");
    return resultmsg;
}

/**
 * \brief Takes all last measurements of the specified device.
 *
 * Device is specified by its t_bios_discovered_device.id_discovered_device.
 *
 * Returns a list of strings. One string for one measurement.
 * Every string has the followng format: "keytag_id:subkeytag_id:value:scale"
 *
 * \param url       - connection url to database.
 * \param device_id - id of the monitor device that was measured.
 * \param name      - output parameter for name.
 *
 * \return NULL            in case of errors.
 *         empty zlist_t   in case of nothing was found.
 *         filled zlist_t  in case of succcess.
 */
// TODO change std:string to char*
zlist_t* select_last_measurements(const char* url, m_dvc_id_t device_id, 
                                  std::string& name)
{
    log_info ("%s \n", "start");
    assert ( device_id ); // is required
    
    zlist_t* measurements = zlist_new();

    zlist_autofree(measurements);


    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            " id_key, value, id_subkey, scale, name"
            " FROM"
            " v_bios_client_info_measurements_last"
            " WHERE id_discovered_device=:deviceid"
        );
    
        // TODO set
        tntdb::Result result = st.setUnsigned32("deviceid", device_id).
                                  select();
        auto rsize = result.size();
        log_debug ("was %d rows selected", rsize);

        // TODO move to constant
        char buff[35];     // 5+5+5+20 
            
        // Go through the selected measurements
        for ( auto &row: result )
        {
            // type_id, required
            m_msrmnt_tp_id_t type_id = 0;
            row[0].get(type_id);
            assert ( type_id );         // database is corrupted
            
            // value, required
            m_msrmnt_value_t value = 0;
            bool isNotNull = row[1].get(value);
            assert ( isNotNull );       // database is corrupted

            // subtype_id, required
            m_msrmnt_sbtp_id_t subtype_id = 0;
            row[2].get(subtype_id);
            assert ( subtype_id );      // database is corrupted

            // scale
            m_msrmnt_scale_t scale = 0;
            isNotNull = row[3].get(scale);
            assert ( isNotNull );           // database is corrupted

            // name
            row[4].get(name);
            assert ( !name.empty() );   // database is corrupted
            
            // TODO use apropriate format strings
            sprintf(buff, "%d:%d:%ld:%d", type_id, subtype_id, 
                          value, scale);
            zlist_push (measurements, buff);
        }
    }
    catch (const std::exception &e) {
        zlist_destroy (&measurements);
        log_warning ("abnormal %s \n", "end");
        return NULL;
    }
    log_info ("normal %s \n", "end");
    return measurements;
}

/**
 * \brief This function processes the COMMON_MSG_GET_LAST_MEASUREMENTS 
 * message.
 *
 * For the correct message processing all message fields should be set up 
 * according specification.
 * 
 * In case of success it generates COMMON_MSG_RETURN_LAST_MEASUREMENTS 
 * message. 
 * In case of failure returns COMMON_MSG_FAIL message.
 * 
 * It doesn't destroy the getmsg.
 *
 * \param url    - the connection to database.
 * \param getmsg - the message of the type COMMON_MSG_GET_LAST_MEASUREMENTS 
 *                  we would like to process.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       COMMON_MSG_RETURN_LAST_MEASUREMENTS message.
 */
zmsg_t* _get_last_measurements(const char* url, common_msg_t* getmsg)
{
    log_info ("%s \n", "start");
    assert ( getmsg );
    assert ( common_msg_id (getmsg) == COMMON_MSG_GET_LAST_MEASUREMENTS );
    
    a_elmnt_id_t device_id = common_msg_device_id (getmsg);
    if ( device_id == 0 )
    {
        log_info ("specifed id  is invalid %s \n", "end");
        return  common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_BADINPUT, 
                     "Invalid element_id for device requested" , NULL);
    }

    m_dvc_id_t device_id_monitor = 0;
    try{
        device_id_monitor = convert_asset_to_monitor(url, device_id);
    }
    catch (const bios::MonitorCounterpartNotFound &e){
        log_info ("monitor counterpart notfound %s \n", "end");
        return  common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_DBCORRUPTED, 
                                                        e.what(), NULL);
    }
    catch (const bios::InternalDBError &e) {
        log_warning ("abnormal %s \n", "end");
        return  common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
    }
    catch (const bios::NotFound &e){
        log_info ("asset element notfound %s \n", "end");
        return  common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_BADINPUT, 
                                                        e.what(), NULL);
    }
    catch (const bios::ElementIsNotDevice &e) {
        log_info ("is not a device %s \n", "end");
        return  common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_BADINPUT, 
                                                        e.what(), NULL);
    }
    std::string device_name = "";
    zlist_t* last_measurements = 
            select_last_measurements(url, device_id_monitor, device_name);
    if ( last_measurements == NULL )
    {
        log_warning ("abnormal %s \n", "end");
        return  common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
            "error during selecting last measurements occured" , NULL);
    }
    else if ( zlist_size (last_measurements) == 0 )
    {
        zlist_destroy (&last_measurements);
        log_info ("notfound %s \n", "end");
        return  common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_NOTFOUND, 
                "for the specified device the is no any measurements" , NULL);
    }
    else
    {
        zmsg_t* return_measurements = 
            common_msg_encode_return_last_measurements(
                device_id,
                device_name.c_str(),
                last_measurements);
        zlist_destroy (&last_measurements);
        log_info ("normal %s \n", "end");
        return return_measurements;
    }
}

/**
 * \brief A wrapper for _get_last_measurements.
 *
 * It do destroy the getmsg.
 *
 * \return zmsg_t - an encoded COMMON_MSG_FAIL or
 *                       COMMON_MSG_RETURN_LAST_MEASUREMENTS message.
 */
zmsg_t* get_last_measurements(zmsg_t** getmsg) {
    log_info ("%s \n", "start");
    common_msg_t *req = common_msg_decode(getmsg);
    zmsg_t *rep = _get_last_measurements(url.c_str(), req);
    common_msg_destroy(&req);
    log_info ("normal %s \n", "end");
    return rep;
}
