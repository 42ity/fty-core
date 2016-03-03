/*
 *
 * Copyright (C) 2015 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*!
 * \file measurement.cc
 * \author Michal Vyskocil <MichalVyskocil@Eaton.com>
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \author Tomas Halman <TomasHalman@Eaton.com>
 * \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 * \author GeraldGuillaume <GeraldGuillaume@Eaton.com>
 * \brief Not yet documented file
 */
#include <inttypes.h>
#include <tntdb.h>
#include <stdexcept>

#include "log.h"
#include "measurement.h"
#include "dbpath.h"
#include "defs.h"

namespace persist {


db_reply_t
    insert_into_measurement(
        tntdb::Connection &conn,
        const char        *topic,
        m_msrmnt_value_t   value,
        m_msrmnt_scale_t   scale,
        int64_t            time,
        const char        *units,
        const char        *device_name,
        TopicCache& c)
{
    db_reply_t ret = db_reply_new();
    

    if ( !units ) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "NULL value of units is not allowed";
        log_error("end: %s", ret.msg.c_str());
        return ret;
    }

    if ( !topic || topic[0]=='@') {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "NULL or malformed value of topic is not allowed";
        log_error("end: %s", ret.msg.c_str());
        return ret;
    }

    // ATTENTION: now name is taken from t_bios_discovered_device
    if ( !device_name ) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "NULL value of device name is not allowed";
        log_error("end: %s", ret.msg.c_str());
        return ret;
    }
    
    try {
        tntdb::Statement st;
        
        m_msrmnt_tpc_id_t topic_id = prepare_topic(conn, topic, units, device_name, c);
        if(topic_id!=0){
            st = conn.prepareCached (
                    "INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) "
                    "  VALUES (:time, :value, :scale, :topic_id)"
                    "  ON DUPLICATE KEY UPDATE value = :value, scale = :scale");
            ret.affected_rows = st.set("time",  time)
                                  .set("value", value)
                                  .set("scale", scale)
                                  .set("topic_id", topic_id)
                                  .execute();

            log_debug("[t_bios_measurement]: inserted %" PRIu64 " rows "\
                       "value:%" PRIi32 " * 10^%" PRIi16 " %s "\
                       "topic = '%s' topic_id=%" PRIi16 " time = %" PRIu64, 
                       ret.affected_rows, value, scale, units, topic, topic_id, time);
        }else{
            ret.status     = 0;
            ret.errtype    = DB_ERR;
            ret.errsubtype = DB_ERROR_INTERNAL;
            ret.msg        = "Can't prepare a valid topic";
            log_error ("NOT INSERTED: value:%" PRIi32 " * 10^%" PRIi16 " %s "\
                       "topic = '%s' time = %" PRIu64, 
                        value, scale, units, topic, time);    
            return ret;
        }

        ret.rowid = conn.lastInsertId();
        ret.status = 1;
        return ret;
    } catch(const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        log_error ("NOT INSERTED: value:%" PRIi32 " * 10^%" PRIi16 " %s "\
                   "topic = '%s' time = %" PRIu64, 
                   value, scale, units, topic, time);
        LOG_END_ABNORMAL(e);
        return ret;
    }
}


// backward compatible function for a case where no cache is required
db_reply_t
    insert_into_measurement(
        tntdb::Connection &conn,
        const char        *topic,
        m_msrmnt_value_t   value,
        m_msrmnt_scale_t   scale,
        int64_t            time,
        const char        *units,
        const char        *device_name)
{
    TopicCache c{};
    return insert_into_measurement(conn, topic, value, scale, time, units, device_name, c);
}

/*
 * add a new device in t_bios_discovered_device table and upate t_bios_monitor_asset_relation 
 * if the device is known in  t_bios_asset_element
 * return id_discovered_device or 0 when not inserted
 */
m_dvc_id_t
    insert_as_not_classified_device(
        tntdb::Connection &conn,
        const char        *device_name)
{
    if(device_name == NULL || device_name[0] == 0 ){
        log_error("[t_bios_discovered_device] can't insert a device with NULL or non device name");
        return 0;
    }
    m_dvc_id_t id_discovered_device=0;
    tntdb::Statement st;
    st = conn.prepareCached(
        " INSERT INTO"
        "   t_bios_discovered_device"
        "     (name, id_device_type)"
        " SELECT"
        "   :name,"
        "   (SELECT T.id_device_type FROM t_bios_device_type T WHERE T.name = 'not_classified')"
        " FROM"
        "   ( SELECT NULL name, 0 id_device_type ) tbl"
        " WHERE :name NOT IN (SELECT name FROM t_bios_discovered_device )"
     );
    uint32_t n = st.set("name", device_name).execute();
    id_discovered_device=conn.lastInsertId();
    log_debug("[t_discovered_device]: device '%s' inserted %" PRIu32 " rows ",
              device_name, n);
    if( n == 1 ) {
        // try to update  relation table
        st = conn.prepareCached(
            " INSERT INTO"
            "   t_bios_monitor_asset_relation (id_discovered_device, id_asset_element)"
            " SELECT"
            "   DD.id_discovered_device, AE.id_asset_element"
            " FROM"
            "   t_bios_discovered_device DD INNER JOIN t_bios_asset_element AE on DD.name = AE.name"
            " WHERE"
            "   DD.name = :name AND"
            "   DD.id_discovered_device NOT IN ( SELECT id_discovered_device FROM t_bios_monitor_asset_relation )"
        );
        n = st.set("name", device_name).execute();
        log_debug("[t_bios_monitor_asset_relation]: inserted %" PRIu32 " rows about %s", n, device_name);
    }else{
        log_error("[t_discovered_device]:  device %s not inserted", device_name );
    }
    return id_discovered_device;
}
//return id_discovered_device or 0 in case of issue
m_dvc_id_t 
    prepare_discovered_device(
        tntdb::Connection &conn,
        const char        *device_name)
{
    //check device name
    if(device_name == NULL || device_name[0] == 0 ){
        log_error("can't prepare a NULL or non device name");
        return 0;
    }
    // verify if the device name exists in t_bios_discovered_device
    // if not create it as not_classified device type
    m_dvc_id_t id_discovered_device;
    tntdb::Statement st = conn.prepareCached(
            " SELECT id_discovered_device "
            " FROM "
            "    t_bios_discovered_device v"
            " WHERE "
            "   v.name = :name");
    st.set("name", device_name);
    try{
        tntdb::Row row=st.selectRow();
        row["id_discovered_device"].get(id_discovered_device);
        return id_discovered_device;
    }catch(const tntdb::NotFound &e){
        log_debug("[t_bios_discovered_device] device %s not found => try to create it as not classified",device_name);
        //add the device as 'not_classified' type
        // probably device doesn't exist in t_bios_discovered_device. Let's fill it.
        return insert_as_not_classified_device(conn,device_name);    
    }
}
//return topic_id or 0 in case of issue
m_msrmnt_tpc_id_t
    prepare_topic(
        tntdb::Connection &conn,
        const char        *topic,
        const char        *units,
        const char        *device_name,
        TopicCache& c)
{
    // is it in the topic cache ?
    if (  c.has(topic) ){
        return c.get(topic);
    }
    //check device name
    if(device_name == NULL || device_name[0] == 0 ){
        log_error("can't prepare a topic %s about a NULL or non device name", topic);
        return 0;
    }
    m_dvc_id_t id_discovered_device=prepare_discovered_device(conn,device_name);    
    if(id_discovered_device==0)return 0;

    m_msrmnt_tpc_id_t topic_id;
    tntdb::Statement st =  conn.prepareCached(
            " INSERT INTO"
            "   t_bios_measurement_topic"
            "    (topic, units, device_id)"
            " VALUES (:topic, :units, :device_id)"
            " ON DUPLICATE KEY"
            "   UPDATE"
            "      id = LAST_INSERT_ID(id)"
    );
    uint32_t n = st.set("topic", topic)
                   .set("units", units)
                   .set("device_id", id_discovered_device)
                   .execute();
    topic_id=conn.lastInsertId();
    if(topic_id!=0){
        c.add(topic,topic_id);
        log_debug("[t_bios_measurement_topic]: inserted topic %s, #%" PRIu32 " rows , topic_id %u", topic, n, topic_id);
    }else{
        log_error("[t_bios_measurement_topic]:  topic %s not inserted", topic);
    }
    return topic_id;
    
}


} //namespace persist
