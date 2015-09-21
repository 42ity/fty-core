/*
Copyright (C) 2015 Eaton
 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <czmq.h>
#include <tntdb/connect.h>
#include <tntdb/row.h>
#include <tntdb/error.h>
#include <tntdb/value.h>
#include <tntdb/result.h>

#include "monitor.h"
#include "log.h"
#include "dbpath.h"
#include "defs.h"
#include "preproc.h"
#include "persist_error.h"
#include "dbhelpers.h"
#include "cleanup.h"

common_msg_t* generate_db_fail(uint32_t errorid, const char* errmsg, 
                               zhash_t** erraux)
{
    log_info ("start");
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_FAIL);
    common_msg_set_errtype (resultmsg, BIOS_ERROR_DB);
    common_msg_set_errorno (resultmsg, errorid);
    common_msg_set_errmsg  (resultmsg, "%s", (errmsg ? errmsg:"") );
    if ( erraux != NULL )
    {
        common_msg_set_aux  (resultmsg, erraux);
        zhash_destroy (erraux);
    }
    log_info ("end: normal");
    return resultmsg;
}

common_msg_t* generate_ok(uint64_t rowid, zhash_t **aux)
{
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_DB_OK);
    common_msg_set_rowid (resultmsg, rowid);
    if ( aux != NULL )
    {
        common_msg_set_aux  (resultmsg, aux);
        zhash_destroy (aux);
    }
    log_info ("db_ok generated for %" PRIu64 , rowid);
    return resultmsg;
}


///////////////////////////////////////////////////////////////////
///////            DEVICE TYPE              ///////////////////////
///////////////////////////////////////////////////////////////////

common_msg_t* generate_device_type(const char* device_type_name)
{
    log_info ("start");
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_DEVICE_TYPE);
    common_msg_set_name (resultmsg, device_type_name);
    log_info ("end: normal");
    return resultmsg;
}

common_msg_t* generate_return_device_type(m_dvc_tp_id_t device_type_id, 
                                          common_msg_t** device_type)
{
    log_info ("start");
    assert ( device_type );
    assert ( *device_type );
    assert ( common_msg_id (*device_type) == COMMON_MSG_DEVICE_TYPE );
    
    _scoped_zmsg_t* nnmsg = common_msg_encode (device_type);
    assert ( nnmsg );
   
    common_msg_t* resultmsg = common_msg_new (COMMON_MSG_RETURN_DEVTYPE);
    common_msg_set_rowid (resultmsg, device_type_id); 
    common_msg_set_msg (resultmsg, &nnmsg);

    log_info ("end: normal");
    return resultmsg;
}

common_msg_t* select_device_type(const char* url, 
                                 const char* device_type_name)
{
    log_info ("start");
    
    if ( !is_ok_name (device_type_name) )
    {
        log_info ("end: too long device type name");
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
        log_info ("end: nothing was found");
        return generate_db_fail (DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        log_warning ("end: abnormal with '%s'", e.what());
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    _scoped_common_msg_t* device_type = generate_device_type (device_type_name);
    log_info ("end: normal");
    return generate_return_device_type (device_type_id, &device_type);
}

common_msg_t* select_device_type(const char* url, 
                                 m_dvc_tp_id_t device_type_id)
{
    log_info ("start");
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
        
        tntdb::Value val = st.set("id", device_type_id).
                              selectValue();
        val.get(device_type_name);
        assert ( !device_type_name.empty() );
    }
    catch (const tntdb::NotFound &e){
        log_info ("end: nothing was found");
        return generate_db_fail(DB_ERROR_NOTFOUND, e.what(), NULL);
    }
    catch (const std::exception &e) {
        log_warning ("end: abnormal with '%s'", e.what());
        return generate_db_fail(DB_ERROR_INTERNAL, e.what(), NULL);
    }
    _scoped_common_msg_t* device_type = generate_device_type 
                                            (device_type_name.c_str());
    log_info ("end: normal");
    return generate_return_device_type (device_type_id, &device_type);
}

common_msg_t* insert_device_type(const char* url, 
                                 const char* device_type_name)
{
    log_info ("start");
   
    if ( !is_ok_name (device_type_name) )
    {
        log_info ("end: too long device type name");
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
    
        n  = st.set("name", device_type_name).
                execute();
        log_debug ("was inserted %" PRIu32 " rows", n);
        newid = conn.lastInsertId();
    }
    catch (const std::exception &e) {
        log_warning ("end: abnormal");
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
    {
        log_info ("end: normal");
        return generate_ok (newid, NULL);
    }
    else
    {
        log_info ("end: nothing was inserted");
        return generate_db_fail (DB_ERROR_BADINPUT, 
                                            "nothing was inserted", NULL);
    }
}

common_msg_t* delete_device_type(const char* url, 
                                 m_dvc_tp_id_t device_type_id)
{
    log_info ("start");
    m_dvc_tp_id_t n = 0;
    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            " v_bios_device_type "
            " WHERE id = :id"
        );
    
        n  = st.set("id", device_type_id).
                execute();
        log_debug ("was deleted %" PRIu32 " rows", n);
    } 
    catch (const std::exception &e) {
        log_warning ("end: abnormal with '%s'", e.what());
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
    {
        log_info ("end: normal");
        return generate_ok (device_type_id, NULL);
    }
    else
    {
        log_info ("end: nothing was deleted");
        return generate_db_fail (DB_ERROR_BADINPUT, 
                                            "nothing was deleted", NULL);
    }
}

common_msg_t* update_device_type(const char* url, 
                                 m_dvc_tp_id_t device_type_id, 
                                 common_msg_t** device_type)
{
    log_info ("start");
    assert ( common_msg_id (*device_type) == COMMON_MSG_DEVICE_TYPE );
    const char* device_type_name = common_msg_name (*device_type);

    if ( !is_ok_name (device_type_name) )
    {
        common_msg_destroy (device_type);
        log_info ("end: too long device type name");
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
        n  = st.set("name", device_type_name).
                set("id", device_type_id).
                execute();
        log_debug ("was updated %" PRIu16 " rows", n);
    }
    catch (const std::exception &e) {
        common_msg_destroy (device_type);
        log_warning ("end: abnormal");
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    common_msg_destroy (device_type);
    if ( n == 1 )
    {
        log_info ("end: normal");
        return generate_ok (device_type_id, NULL);
    }
    else
    {   
        log_info ("end: nothing was updated");
        return generate_db_fail (DB_ERROR_BADINPUT, 
                                        "nothing was updated", NULL);
    }
}


////////////////////////////////////////////////////////////////////////
/////////////////           DEVICE                   ///////////////////
////////////////////////////////////////////////////////////////////////
// devices should have a unique names
db_reply_t 
    select_device (tntdb::Connection &conn, 
                   const char* device_name)
{
    LOG_START;
    
    db_reply_t ret = db_reply_new();
 
    if ( !is_ok_name (device_name) )
    {
        ret.status     = 0; 
        log_info ("end: too long device name");
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "device name length is not in range [1, MAX_NAME_LENGTH]";
        return ret;
    }

    try{
        // ASSUMPTION: devices have unique names
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "   v.id"
            " FROM"
            "   v_bios_discovered_device v"
            " WHERE "
            "   v.name = :name"
            " LIMIT 1"
        );
        
        tntdb::Row row = st.set("name", device_name).
                            selectRow();
        log_debug ("1 row was selected");
        
        row[0].get(ret.item);

        ret.status = 1;

        LOG_END;
        return ret;
    }
    catch (const tntdb::NotFound &e){
        ret.status     = 0; 
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_NOTFOUND;
        ret.msg        = e.what();
        log_info ("end: discovered device was not found with '%s'", e.what());
        return ret;
    }
    catch (const std::exception &e) {
        ret.status     = 0; 
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL (e);
        return ret;
    }
}

////////////////////////////////////////////////////////////////////////
/////////////////           MEASUREMENT              ///////////////////
////////////////////////////////////////////////////////////////////////


common_msg_t* test_insert_measurement(const char    *url, 
                                 m_dvc_id_t          device_id, 
                                 const char         *topic, 
                                 m_msrmnt_value_t    value,
                                 m_msrmnt_scale_t    scale,
                                 uint32_t seconds)
{
    assert ( device_id );    // is required (if not device was measured, 
                             // then use "dummy_monitor_device") 

    m_msrmnt_id_t n     = 0;     // number of rows affected.
    m_msrmnt_id_t newid = 0;

    try{
        tntdb::Connection conn = tntdb::connectCached(url);

        tntdb::Statement st_topic = conn.prepareCached(
            " INSERT INTO"
            "   t_bios_measurement_topic"
            "     (topic, units, device_id)"
            " VALUES"
            "   (:topic, 'aa', :device)"
            " ON DUPLICATE KEY"
            "   UPDATE"
            "       id = LAST_INSERT_ID(id)"
        );

        // Insert one row or nothing
        n  = st_topic.set("topic", topic).
                      set("device", device_id).
                      execute();
        log_info ("[t_bios_measurement_topic]: was inserted %" PRIu64 " rows", n);
        m_msrmnt_tpc_id_t topic_id = conn.lastInsertId();

        tntdb::Statement st_meas = conn.prepareCached(
            " INSERT INTO"
            " t_bios_measurement"
            "   (topic_id, value, scale, timestamp)"
            " VALUES"
            "   (:topicid, :val, :scale, :seconds)" 
        );

        // Insert one row or nothing
        n  = st_meas.set("topicid", topic_id).
                     set("scale", scale).
                     set("val", value).
                     set("seconds", time(NULL) - seconds).
                     execute();
        log_info ("[t_bios_measurement]: was inserted %" PRIu64 " rows", n);

        newid = conn.lastInsertId();
    }
    catch (const std::exception &e) {
        log_warning ("end: abnormal with '%s'", e.what());
        return generate_db_fail (DB_ERROR_INTERNAL, e.what(), NULL);
    }
    if ( n == 1 )
    {
        return generate_ok (newid, NULL);
    }
    else
    {
        // TODO need to return existing ID????
        log_info ("end: nothing was inserted");
        return generate_db_fail (DB_ERROR_NOTHINGINSERTED, 
                                                "nothing was inserted", NULL);
    }
}


void generate_measurements (const char      *url, 
                            m_dvc_id_t       device_id, 
                            uint32_t         max_seconds, 
                            m_msrmnt_value_t last_value)
{
    auto ret UNUSED_PARAM = test_insert_measurement (url, device_id, "realpower.default" , -9,-1, max_seconds + 10);
    
    for ( int i = 1; i < GEN_MEASUREMENTS_MAX ; i++ )
        ret = test_insert_measurement (url, device_id, "realpower.default", device_id + i, -1, max_seconds - i);
    ret = test_insert_measurement (url, device_id, "realpower.default", last_value, -1, max_seconds - GEN_MEASUREMENTS_MAX);
}

zlist_t* select_last_measurements (tntdb::Connection &conn, m_dvc_id_t device_id, std::string &device_name)
{
    LOG_START;
    log_debug ("device_id (monitor) = %" PRIu32, device_id);
    assert ( device_id ); // is required
    
    zlist_t* measurements = zlist_new();
    zlist_autofree(measurements);

    try{
        tntdb::Statement st_name = conn.prepareCached(
            " SELECT"
            "   v1.name"
            " FROM"
            "   t_bios_discovered_device v1"
            " WHERE v1.id_discovered_device = :deviceid"
        );
    
        tntdb::Row row = st_name.set("deviceid", device_id).
                                 selectRow();
        log_debug ("[t_discovered_device] was %u rows selected", 1);


        row[0].get(device_name);
    }
    catch (const std::exception &e) {
        device_name = "";
        zlist_destroy (&measurements);
        LOG_END_ABNORMAL(e);
        return NULL;
    }
    try{
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "   v.value, v.scale, v.topic"
            " FROM"
            "   v_web_measurement_last_10m v"
            " WHERE v.device_id = :deviceid"
        );
    
        tntdb::Result result = st.set("deviceid", device_id).
                                  select();
        auto rsize = result.size();
        log_debug ("was %u rows selected", rsize);

        // Go through the selected measurements
        for ( auto &row: result )
        {          
            // value, required
            m_msrmnt_value_t value = 0;
            bool isNotNull = row[0].get(value);
            assert ( isNotNull );       // database is corrupted

            // scale
            m_msrmnt_scale_t scale = 0;
            isNotNull = row[1].get(scale);
            assert ( isNotNull );           // database is corrupted

            // topic
            std::string topic = "";
            row[2].get(topic);
            assert ( !topic.empty() );   // database is corrupted
            
            zlist_push (measurements, (char *)( 
                             std::to_string (value) + ":" +
                             std::to_string (scale) + ":" + topic
                             ).c_str());
        }
        

    }
    catch (const std::exception &e) {
        zlist_destroy (&measurements);
        LOG_END_ABNORMAL(e);
        return NULL;
    }
    LOG_END;
    return measurements;
}

zmsg_t* _get_last_measurements(const char* url, common_msg_t* getmsg)
{
    log_info ("start");
    assert ( getmsg );
    assert ( common_msg_id (getmsg) == COMMON_MSG_GET_LAST_MEASUREMENTS );
    
    a_elmnt_id_t device_id = common_msg_device_id (getmsg);
    if ( device_id == 0 )
    {
        log_info ("end: specifed id is invalid");
        return  common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_BADINPUT, 
                     "Invalid element_id for device requested" , NULL);
    }

    // TODO it in better way
    tntdb::Connection conn = tntdb::connectCached(url);

    m_dvc_id_t device_id_monitor = 0;
    try{
        device_id_monitor = convert_asset_to_monitor_old(url, device_id);
    }
    catch (const bios::MonitorCounterpartNotFound &e){
        log_info ("end: monitor counterpart notfoun");
        return  common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_DBCORRUPTED, 
                                                        e.what(), NULL);
    }
    catch (const bios::InternalDBError &e) {
        log_warning ("end: abnormal with '%s'", e.what());
        return  common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                                        e.what(), NULL);
    }
    catch (const bios::NotFound &e){
        log_info ("end: asset element notfound with '%s'", e.what());
        return  common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_NOTFOUND,
                                                        e.what(), NULL);
    }
    catch (const bios::ElementIsNotDevice &e) {
        log_info ("end: is not a device");
        return  common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_BADINPUT, 
                                                        e.what(), NULL);
    }
    std::string device_name = "";
    _scoped_zlist_t* last_measurements = 
            select_last_measurements(conn, device_id_monitor, device_name);
    // TODO take care about it
    if ( last_measurements == NULL )
    {
        log_warning ("end: abnormal");
        return  common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
            "error during selecting last measurements occured" , NULL);
    }
    else
    {
        zmsg_t* return_measurements = 
            common_msg_encode_return_last_measurements(
                device_id,
                device_name.c_str(),
                last_measurements);
        zlist_destroy (&last_measurements);
        log_info ("end: normal");
        return return_measurements;
    }
}

zmsg_t* get_last_measurements(zmsg_t** getmsg) {
    log_info ("start");
    _scoped_common_msg_t *req = common_msg_decode(getmsg);
    zmsg_t *rep = _get_last_measurements(url.c_str(), req);
    common_msg_destroy(&req);
    log_info ("end: normal");
    return rep;
}
