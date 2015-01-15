/*
Copyright (C) 2015 Eaton
 
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

/*! \file monitor.cc
    \brief Functions for manipulating with elements in database monitor part.
    \author Alena Chernikava <alenachernikava@eaton.com>
*/

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

bool is_ok_name_length (const char* name)
{
    size_t length = strlen (name);
    if ( ( length == 0 ) || ( length > MAX_NAME_LENGTH ) )
        return false;
    else 
        return true;
}

common_msg_t* generate_db_fail(uint32_t errorid, const char* errmsg, 
                               zhash_t** erraux)
{
    log_info ("%s \n", "start");
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_FAIL);
    common_msg_set_errtype (resultmsg, BIOS_ERROR_DB);
    common_msg_set_errorno (resultmsg, errorid);
    common_msg_set_errmsg  (resultmsg, "%s", (errmsg ? errmsg:"") );
    if ( erraux != NULL )
    {
        common_msg_set_erraux  (resultmsg, erraux);
        zhash_destroy (erraux);
    }
    log_info ("normal %s \n", "end");
    return resultmsg;
}

common_msg_t* generate_ok(uint64_t rowid)
{
    log_info ("%s \n", "start");
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_DB_OK);
    common_msg_set_rowid (resultmsg, rowid);
    log_info ("normal %s \n", "end");
    return resultmsg;
}

////////////////////////////////////////////////////////////////////
///////            CLIENT                    ///////////////////////
////////////////////////////////////////////////////////////////////

common_msg_t* generate_client(const char* client_name)
{
    log_info ("%s \n", "start");
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_CLIENT);
    common_msg_set_name (resultmsg, client_name);
    log_info ("normal %s \n", "end");
    return resultmsg;
}

common_msg_t* generate_return_client(m_clnt_id_t client_id, 
                                     common_msg_t** client)
{
    log_info ("%s \n", "start");
    assert ( client );
    assert ( *client );
    assert ( common_msg_id (*client) == COMMON_MSG_CLIENT );
   
    zmsg_t* nnmsg = common_msg_encode (client);
    assert ( nnmsg );
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_RETURN_CLIENT);
    common_msg_set_rowid (resultmsg, client_id);
    common_msg_set_msg (resultmsg, &nnmsg);
    log_info ("normal %s \n", "end");
    return resultmsg;
}

common_msg_t* select_client(const char* url, const char* client_name)
{
    log_info ("%s \n", "start");
    
    if ( !is_ok_name_length (client_name) )
    {
        log_info ("too long client name %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
        "client name length is not in range [1, MAX_NAME_LENGTH]", NULL);
    }

    m_clnt_id_t client_id = 0;

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
          
        tntdb::Value val = st.setString("name", client_name).
                              selectValue();
        val.get(client_id);
        assert ( client_id );
    }
    catch (const tntdb::NotFound &e){
        log_info ("nothing was found %s \n", "end");
        return generate_db_fail (DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        log_warning ("abnormal %s \n", "end");
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_t* client = generate_client (client_name);
    log_info ("normal %s \n", "end");
    return generate_return_client (client_id, &client);
}

common_msg_t* select_client(const char* url, m_clnt_id_t client_id)
{
    log_info ("%s \n", "start");
    std::string client_name = "";
    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            " v.name"
            " FROM"
            " v_bios_client v"
            " WHERE v.id = :id"
        );
          
        tntdb::Value val = st.set("id", client_id).
                              selectValue();
        val.get(client_name);
        assert ( !client_name.empty() );
    }
    catch (const tntdb::NotFound &e){
        log_info ("nothing was found %s \n", "end");
        return generate_db_fail(DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        log_warning ("abnormal %s \n", "end");
        return generate_db_fail(DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_t* client = generate_client (client_name.c_str());
    log_info ("normal %s \n", "end");
    return generate_return_client (client_id, &client);
}

common_msg_t* insert_client(const char* url, const char* client_name)
{
    log_info ("%s \n", "start");

    if ( !is_ok_name_length (client_name) )
    {
        log_info ("too long client name %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
        "client name length is not in range [1, MAX_NAME_LENGTH]", NULL);
    }

    m_clnt_id_t n = 0;
    m_clnt_id_t newid = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            " v_bios_client (id,name)"
            " VALUES (NULL,:name)"
        );
    
        n  = st.set("name", client_name).
                execute();

        log_debug("was inserted %d rows \n", n);
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
        log_info ("nothing was inserted %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
                                            "nothing was inserted", NULL);
    }
}

common_msg_t* delete_client(const char* url, m_clnt_id_t client_id)
{
    log_info ("%s \n", "start");
    m_clnt_id_t n = 0;
    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            " v_bios_client "
            " WHERE id = :id"
        );
    
        n  = st.set("id", client_id).
                execute();
        log_debug("was deleted %d rows \n", n);
    } 
    catch (const std::exception &e) {
        log_warning ("abnormal %s \n", "end");
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
    {
        log_info ("normal %s \n", "end");
        return generate_ok (client_id);
    }
    else
    {
        log_info ("nothing was deleted %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
                                            "nothing was deleted", NULL);
    }
}

common_msg_t* update_client(const char* url, m_clnt_id_t client_id, 
                            common_msg_t** client)
{
    log_info ("%s \n", "start");
    assert ( client );
    assert ( *client );
    assert ( common_msg_id (*client) == COMMON_MSG_CLIENT );

    const char* client_name = common_msg_name (*client);
   
    if ( !is_ok_name_length (client_name) )
    {
        log_info ("too long client name %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
        "client name length is not in range [1, MAX_NAME_LENGTH]", NULL);
    }

    m_clnt_id_t n = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " UPDATE"
            " v_bios_client"
            " SET name = :name"
            " WHERE id = :id"
        );
    
        n  = st.set("name", client_name).
                set("id", client_id).
                execute();
        log_debug("was updated %d rows \n", n);
    }
    catch (const std::exception &e) {
        log_warning ("abnormal %s \n", "end");
        common_msg_destroy (client);
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_destroy (client);
    if ( n == 1 )
    {
        log_info ("normal %s \n", "end");
        return generate_ok (client_id);
    }
    else
    {
        log_info ("nothing was updated %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
                                            "nothing was updated", NULL);
    }
}

////////////////////////////////////////////////////////////////////////
/////////////////           CLIENT INFO              ///////////////////
////////////////////////////////////////////////////////////////////////

common_msg_t* generate_client_info
    (m_clnt_id_t client_id, m_dvc_id_t device_id, uint32_t mytime, 
    byte* data, uint32_t datasize)
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

    log_info ("normal %s \n", "end");
    return resultmsg;
}

common_msg_t* generate_return_client_info(m_clnt_info_id_t client_info_id, 
                                          common_msg_t** client_info)
{
    log_info ("%s \n", "start");
    assert ( client_info );
    assert ( *client_info );
    assert ( common_msg_id (*client_info) == COMMON_MSG_CLIENT_INFO );

    zmsg_t* nnmsg = common_msg_encode (client_info);
    assert ( nnmsg );
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_RETURN_CINFO);
    common_msg_set_rowid (resultmsg, client_info_id);
    common_msg_set_msg (resultmsg, &nnmsg);

    log_info ("normal %s \n", "end");
    return resultmsg;
}

common_msg_t* insert_client_info (const char* url, m_dvc_id_t device_id, 
                                  m_clnt_id_t client_id, zchunk_t** blob)
{
    log_info ("%s \n", "start");
    assert ( device_id );  // is required
    assert ( client_id );  // is required
    assert ( blob );
    assert ( *blob );      // is required

    m_clnt_info_id_t n = 0;     // number of rows affected
    m_clnt_info_id_t newid = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            "   v_bios_client_info (id, id_client, id_discovered_device,"
            "                       ext, timestamp)"
            " VALUES (NULL, :idclient, :iddiscovereddevice, :ext,"
            "            UTC_TIMESTAMP())"
        );          // time is the time of inserting into database
        tntdb::Blob blobData ((const char*) zchunk_data(*blob), 
                              zchunk_size (*blob));

        n = st.set("idclient", client_id).
               set("iddiscovereddevice", device_id).
               setBlob("ext", blobData).
               execute();
        log_debug("was inserted %ld rows \n", n);
        newid = conn.lastInsertId();
    }
    catch (const std::exception &e) {
        log_warning ("abnormal %s \n", "end");
        zchunk_destroy (blob);
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    zchunk_destroy (blob);
    if ( n == 1 )
    {
        log_info ("normal %s \n", "end");
        return generate_ok (newid);
    }
    else
    {
        log_info ("nothing was inserted %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
                                            "nothing was inserted", NULL);
    }
}

common_msg_t* insert_client_info  (const char* url, m_dvc_id_t device_id, 
                m_clnt_id_t client_id, byte* blobdata, uint32_t blobsize)
{
    assert ( device_id );         // is required
    assert ( client_id );         // is required
    assert ( blobsize );          // is required

    zchunk_t* blob = zchunk_new (blobdata, blobsize);

    return insert_client_info (url, device_id, client_id, &blob);
}

common_msg_t* delete_client_info (const char* url, 
                                  m_clnt_info_id_t client_info_id)
{
    log_info ("%s \n", "start");
    m_clnt_info_id_t n = 0;
    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            " v_bios_client_info "
            " WHERE id = :id"
        );
    
        n  = st.set("id", client_info_id).execute();
        log_debug ("was deleted %ld rows \n", n);
    }
    catch (const std::exception &e) {
        log_warning ("abnormal %s \n", "end");
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
    {
        log_info ("normal %s \n", "end");
        return generate_ok (client_info_id);
    }
    else
    {
        log_info ("nothing was deleted %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
                                        "nothing was deleted", NULL);
    }
}

common_msg_t* update_client_info
    (const char* url, m_clnt_info_id_t client_info_id, zchunk_t** blob)
{
    log_info ("%s \n", "start");
    assert ( client_info_id );  // is required
    assert ( blob );
    assert ( *blob );      // is required

    m_clnt_info_id_t n = 0;     // number of rows affected

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " UPDATE t_bios_client_info"
            " SET ext=:ext, timestamp=UTC_TIMESTAMP()"
            " WHERE id_client_info=:idclientinfo"
        );          // time is the time of inserting into database
        tntdb::Blob blobData((const char*) zchunk_data(*blob), 
                             zchunk_size (*blob));
        zchunk_destroy (blob);

        n = st.set("idclientinfo", client_info_id).
               setBlob("ext", blobData).
               execute();
        log_debug ("was updated %ld rows \n", n);
    }
    catch (const std::exception &e) {
        log_warning ("abnormal %s \n", "end");
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
    {
        log_info ("normal %s \n", "end");
        return generate_ok (client_info_id);
    }
    else
    {
        log_info ("nothing was updated %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
                                            "nothing was updated", NULL);
    }
}

// update ui_properties
common_msg_t* update_ui_properties
    (const char* url, zchunk_t** blob)
{
    log_info ("%s \n", "start");
    assert ( blob );
    assert ( *blob );      // is required

    m_clnt_id_t n = 0;     // number of rows affected

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " UPDATE t_bios_client_info"
            " SET ext=:ext, timestamp=UTC_TIMESTAMP()"
            " WHERE id_client=:idclient"
        );          // time is the time of inserting into database
        tntdb::Blob blobData((const char*) zchunk_data(*blob), 
                             zchunk_size (*blob));
        zchunk_destroy (blob);

        n = st.set("idclient", UI_PROPERTIES_CLIENT_ID).
               setBlob("ext", blobData).
               execute();
        log_debug ("was updated %d rows \n", n);
    }
    catch (const std::exception &e) {
        log_warning ("abnormal %s \n", "end");
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
    {
        log_info ("normal %s \n", "end");
        return generate_ok (UI_PROPERTIES_CLIENT_ID);
    }
    else
    {
        log_info ("nothing was updated %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
                                            "nothing was updated", NULL);
    }
}

common_msg_t* select_client_info_last(const char  * url, 
                                      m_clnt_id_t client_id, 
                                      m_dvc_id_t  device_id)
{
    log_info ("%s \n", "start");
    assert ( client_id );  // is required
    assert ( device_id );  // is required

    m_clnt_info_id_t id = 0;
    tntdb::Blob myBlob;
    time_t mydatetime = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url); 

        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            " v.id, UNIX_TIMESTAMP(v.datum) as utm, v.info"
            " FROM"
            " v_bios_client_info_last v"
            " WHERE"
            "    v.id_discovered_device = :id_devicediscovered AND"
            "    v.id_client = :id_client"
        );

        // Should return one row or nothing.
        tntdb::Row row = st.set("id_deviceidscovered", device_id).
                            set("id_client", client_id).
                            selectRow();

        row[0].get(id);
        assert ( id );
    
        bool isNotNull = row[1].get(mydatetime);
        assert ( isNotNull );

        isNotNull = row[2].get(myBlob);
        assert ( isNotNull );
    }
    catch (const tntdb::NotFound &e) {
        log_info ("nothing was found %s \n", "end");
        return generate_db_fail (DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        log_warning ("abnormal %s \n", "end");
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    log_info ("normal %s \n", "end");
    return generate_client_info(client_id, device_id, mydatetime, 
                                        (byte*)myBlob.data(), myBlob.size());
}

common_msg_t* select_client_info(const char* url, 
                                 m_clnt_info_id_t client_info_id)
{
    log_info ("%s \n", "start");
    assert ( client_info_id );

    m_clnt_id_t client_id = 0;
    m_dvc_id_t device_id = 0;
    time_t mydatetime = 0;
    tntdb::Blob myBlob;

    try{
        tntdb::Connection conn = tntdb::connectCached(url); 

        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            "   UNIX_TIMESTAMP(v.timestamp) as utm,"
            "   v.ext, v.id_client , v.id_discovered_device"
            " FROM"
            "   v_bios_client_info v"
            " WHERE v.id = :id"
        );
        
        tntdb::Row row = st.set("id", client_info_id).
                            selectRow();
          
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
        log_info ("nothing was found %s \n", "end");
        return generate_db_fail (DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        log_warning ("abnormal %s \n", "end");
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_t* client_info = generate_client_info(client_id, device_id, 
                        mydatetime, (byte*)myBlob.data(), myBlob.size());
    log_info ("normal %s \n", "end");
    return generate_return_client_info (client_info_id, &client_info);
}

common_msg_t* select_ui_properties(const char* url)
{
    log_info ("%s \n", "start");
    
    m_clnt_info_id_t client_info_id = 0;
    m_dvc_id_t device_id = 0;
    time_t mydatetime = 0;
    tntdb::Blob myBlob;

    try{
        tntdb::Connection conn = tntdb::connectCached(url); 

        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            "   UNIX_TIMESTAMP(v.timestamp) AS utm,"
            "   v.ext, v.id , v.id_discovered_device"
            " FROM"
            " v_bios_client_info v"
            " WHERE v.id_client = :id"
        );
        
        tntdb::Row row = st.set("id", UI_PROPERTIES_CLIENT_ID).
                            selectRow();
          
        bool isNotNull = row[0].get(mydatetime);
        assert ( isNotNull );
                               
        isNotNull = row[1].get(myBlob);
        assert ( isNotNull );
    
        row[2].get(client_info_id);
        assert ( client_info_id );
        
        row[3].get(device_id);
        // ui_properties are not tied to *any* device
        assert ( device_id == NULL);
    }
    catch (const tntdb::NotFound &e) {
        log_info ("nothing was found %s \n", "end");
        return generate_db_fail (DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        log_warning ("abnormal %s \n", "end");
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_t* client_info = generate_client_info(
            UI_PROPERTIES_CLIENT_ID, device_id, mydatetime, 
            (byte*)myBlob.data(), myBlob.size());
    log_info ("normal %s \n", "end");
    return generate_return_client_info (client_info_id, &client_info);
}

///////////////////////////////////////////////////////////////////
///////            DEVICE TYPE              ///////////////////////
///////////////////////////////////////////////////////////////////

common_msg_t* generate_device_type(const char* device_type_name)
{
    log_info ("%s \n", "start");
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_DEVICE_TYPE);
    common_msg_set_name (resultmsg, device_type_name);
    log_info ("normal %s \n", "end");
    return resultmsg;
}

common_msg_t* generate_return_device_type(m_dvc_tp_id_t device_type_id, 
                                          common_msg_t** device_type)
{
    log_info ("%s \n", "start");
    assert ( device_type );
    assert ( *device_type );
    assert ( common_msg_id (*device_type) == COMMON_MSG_DEVICE_TYPE );
    
    zmsg_t* nnmsg = common_msg_encode (device_type);
    assert ( nnmsg );
   
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_RETURN_DEVTYPE);
    common_msg_set_rowid (resultmsg, device_type_id); 
    common_msg_set_msg (resultmsg, &nnmsg);

    log_info ("normal %s \n", "end");
    return resultmsg;
}

common_msg_t* select_device_type(const char* url, 
                                 const char* device_type_name)
{
    log_info ("%s \n", "start");
    
    if ( !is_ok_name_length (device_type_name) )
    {
        log_info ("too long device type name %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
        "device type name length is not in range [1, MAX_NAME_LENGTH]", NULL);
    }

    m_dvc_tp_id_t device_type_id = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            " v.id"
            " FROM"
            " v_bios_device_type v"
            " WHERE v.name = :name"
        );
          
        tntdb::Value val = st.setString("name", device_type_name).
                              selectValue();
        val.get(device_type_id);
        assert (device_type_id);
    }
    catch (const tntdb::NotFound &e){
        log_info ("nothing was found %s \n", "end");
        return generate_db_fail (DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        log_warning ("abnormal %s \n", "end");
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_t* device_type = generate_device_type (device_type_name);
    log_info ("normal %s \n", "end");
    return generate_return_device_type (device_type_id, &device_type);
}

common_msg_t* select_device_type(const char* url, 
                                 m_dvc_tp_id_t device_type_id)
{
    log_info ("%s \n", "start");
    std::string device_type_name = "";
    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            " v.name"
            " FROM"
            " v_bios_device_type v"
            " WHERE v.id = :id"
        );
        
        // TODO set  
        tntdb::Value val = st.set("id", device_type_id).
                              selectValue();
        val.get(device_type_name);
        assert ( !device_type_name.empty() );
    }
    catch (const tntdb::NotFound &e){
        log_info ("nothing was found %s \n", "end");
        return generate_db_fail(DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        log_warning ("abnormal %s \n", "end");
        return generate_db_fail(DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_t* device_type = generate_device_type 
                                            (device_type_name.c_str());
    log_info ("normal %s \n", "end");
    return generate_return_device_type (device_type_id, &device_type);
}

common_msg_t* insert_device_type(const char* url, 
                                 const char* device_type_name)
{
    log_info ("%s \n", "start");
   
    if ( !is_ok_name_length (device_type_name) )
    {
        log_info ("too long device type name %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
        "device type name length is not in range [1, MAX_NAME_LENGTH]", NULL);
    }

    m_dvc_tp_id_t n = 0;
    m_dvc_tp_id_t newid = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            " v_bios_device_type (id,name)"
            " SELECT NULL, :name FROM DUAL WHERE NOT EXISTS"
            " (SELECT id FROM v_bios_device_type WHERE name=:name)"
        );
    
        // TODO set
        n  = st.set("name", device_type_name).
                execute();
        log_debug ("was inserted %d rows", n);
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
        log_info ("nothing was inserted %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
                                            "nothing was inserted", NULL);
    }
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
        log_debug ("was deleted %d rows", n);
    } 
    catch (const std::exception &e) {
        log_warning ("abnormal %s \n", "end");
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
    {
        log_info ("normal %s \n", "end");
        return generate_ok (device_type_id);
    }
    else
    {
        log_info ("nothing was deleted %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
                                            "nothing was deleted", NULL);
    }
}

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
        "device type name length is not in range [1, MAX_NAME_LENGTH]", NULL);
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

common_msg_t* generate_return_device(m_dvc_id_t device_id, 
                                     common_msg_t** device)
{
    log_info ("%s \n", "start");
    assert ( device );
    assert ( *device );
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

common_msg_t* insert_device(const char* url, m_dvc_tp_id_t device_type_id, 
                            const char* device_name)
{
    log_info ("%s \n", "start");
    assert ( device_type_id );       // is required
    
    if ( !is_ok_name_length (device_name) )
    {
        log_info ("too long device name %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
            "device name length is not in range [1, MAX_NAME_LENGTH]", NULL);
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

common_msg_t* update_device (const char* url, common_msg_t** new_device)
{
    return generate_db_fail (DB_ERROR_NOTIMPLEMENTED, NULL, NULL);  
    // TODO NOT IMPLEMENTED
}

common_msg_t* select_device (const char* url, m_dvc_tp_id_t device_type_id, 
                             const char* device_name)
{
    log_info ("%s \n", "start");
    assert ( device_type_id );
    
    if ( !is_ok_name_length (device_name) )
    {
        log_info ("too long device name %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
            "device name length is not in range [1, MAX_NAME_LENGTH]", NULL);
    }

    try{
        tntdb::Connection conn = tntdb::connectCached(url); 

        // There are discovered devices with such name
        // TODO for now take first in the list
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

common_msg_t* select_device (const char* url, const char* device_type_name, 
                             const char* device_name)
{
    log_info ("%s \n", "start");
 
    if ( !is_ok_name_length (device_name) )
    {
        log_info ("too long device name %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
            "device name length is not in range [1, MAX_NAME_LENGTH]", NULL);
    }
    if ( !is_ok_name_length (device_type_name) )
    {
        log_info ("too long device type name %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
        "device type name length is not in range [1, MAX_NAME_LENGTH]", NULL);
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

common_msg_t* insert_device(const char* url, const char* device_type_name, 
                            const char* device_name)
{
    log_info ("%s \n", "start");
    
    if ( !is_ok_name_length (device_name) )
    {
        log_info ("too long device name %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
            "device name length is not in range [1, MAX_NAME_LENGTH]", NULL);
    }
    if ( !is_ok_name_length (device_type_name) )
    {
        log_info ("too long device type name %s \n", "end");
        return generate_db_fail (DB_ERROR_BADINPUT, 
        "device type name length is not in range [1, MAX_NAME_LENGTH]", NULL);
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

common_msg_t* insert_measurement(const char         *url, 
                                 m_clnt_id_t        client_id, 
                                 m_dvc_id_t         device_id, 
                                 m_msrmnt_tp_id_t   type_id, 
                                 m_msrmnt_sbtp_id_t subtype_id, 
                                 m_msrmnt_value_t   value)
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
        return  common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_NOTFOUND,
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

zmsg_t* get_last_measurements(zmsg_t** getmsg) {
    log_info ("%s \n", "start");
    common_msg_t *req = common_msg_decode(getmsg);
    zmsg_t *rep = _get_last_measurements(url.c_str(), req);
    common_msg_destroy(&req);
    log_info ("normal %s \n", "end");
    return rep;
}
