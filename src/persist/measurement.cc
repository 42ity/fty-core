#include <vector>
#include <tntdb.h>
#include <errno.h>
#include <czmq.h>
#include <zchunk.h>
#include <inttypes.h>

#include "log.h"
#include "measurement.h"
#include "ymsg.h"
#include "log.h"
#include "dbpath.h"
#include "bios_agent.h"
#include "agents.h"
#include "utils_ymsg.h"
#include "cleanup.h"
#include "utils.h"

namespace persist {

bool TopicCache::has(const std::string& topic) const {
    if (_cache.count(topic) == 1)
        return true;

    return false;
}

void TopicCache::add(const std::string& topic) {
    if (_max <= _cache.size())
    {
        _cache.clear();
    }

    _cache.insert(topic);
}

//Notice for future developers - this functions is ugly and better to split to smaller
//functions doing only one thing. Please try to avoid adding even more logic here, thanks
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
        log_error("end: %s", ret.msg);
        return ret;
    }
    
    if ( !topic ) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "NULL value of topic is not allowed";
        log_error("end: %s", ret.msg);
        return ret;
    }
    
    // ATTENTION: now name is taken from t_bios_discovered_device
    if ( !device_name ) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "NULL value of device name is not allowed";
        log_error("end: %s", ret.msg);
        return ret;
    }

    try {
insert_into_measurement_again:
        tntdb::Statement st;

        if (  !c.has(topic) )
        {
            st = conn.prepareCached(
                    " INSERT INTO"
                    "   t_bios_measurement_topic"
                    "    (topic, units, device_id)"
                    " SELECT"
                    "   :topic, :units, v.id_discovered_device"
                    " FROM"
                    "   t_bios_discovered_device v"
                    " WHERE"
                    "   v.name = :name"
                    " ON DUPLICATE KEY"
                    "   UPDATE"
                    "      id = LAST_INSERT_ID(id)"
            );
            uint32_t n = st.set("topic", topic)
                           .set("units", units)
                           .set("name", device_name)
                           .execute();
            log_debug("[t_bios_measurement_topic]: inserted topic %s, #%" PRIu32 " rows ", topic, n);
        }

        st = conn.prepareCached(
                " INSERT INTO"
                "   t_bios_measurement"
                "       (timestamp, value, scale, topic_id)"
                " SELECT"
                "   :time, :value, :scale, id"
                " FROM"
                "   t_bios_measurement_topic"
                " WHERE topic=:topic AND"
                "       units=:units"
        );
        ret.affected_rows = st.set("topic", topic)
                              .set("time",  time)
                              .set("value", value)
                              .set("scale", scale)
                              .set("units", units)
                              .execute();

        log_debug("[t_bios_measurement]: inserted %" PRIu64 " rows "\
                   "value:%" PRIi32 " * 10^%" PRIi16 " %s "\
                   "topic = '%s' time = %" PRIu64, 
                   ret.affected_rows, value, scale, units, topic, time);

        if( ret.affected_rows == 0 && device_name != NULL && device_name[0] != 0 ) {
            // probably device doesn't exist in t_bios_discovered_device. Let's fill it.
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
            log_debug("[t_discovered_device]: device '%s' inserted %" PRIu32 " rows ",
                      device_name ? device_name : "null", n);
            if( n == 1 ) {
                // update also relation table
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
                log_debug("[t_bios_monitor_asset_relation]: inserted %" PRIu32 " rows ", n);
                goto insert_into_measurement_again; // successfully inserted into _discovered_device, save measurement one more time
            }
        }
        else
        {
            c.add(topic);
            log_debug ("topic added to cache");
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

db_reply <std::vector<db_msrmnt_t>>
    select_from_measurement_by_topic(
        tntdb::Connection &conn,
        const char        *topic)
{   
    LOG_START;
    std::vector<db_msrmnt_t> item{};
    db_reply<std::vector<db_msrmnt_t>> ret = db_reply_new(item);

    if ( !topic ) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg = "NULL value of topic is not allowed";
        log_error("end: %s", ret.msg);
        return ret;
    }
    
    try {
        tntdb::Statement st = conn.prepareCached(
                " SELECT id, timestamp, value, scale, device_id, units, topic"
                " FROM v_bios_measurement"
                " WHERE topic LIKE :topic");
        tntdb::Result res = st.set("topic", topic)
                              .select();
        
        log_debug ("was %u rows selected", res.size());

        for ( auto &r : res ) {

            db_msrmnt_t m = {0, 0, 0, 0, 0, "", ""};

            r[0].get(m.id);
            r[1].get(m.timestamp);
            r[2].get(m.value);
            r[3].get(m.scale);
            r[4].get(m.device_id);
            r[5].get(m.units);
            r[6].get(m.topic);

            ret.item.push_back(m);
        }
        ret.status = 1;
    } catch(const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        ret.item.clear();
        LOG_END_ABNORMAL(e);
        return ret;
    }
    LOG_END;
    return ret;
}

db_reply_t
    delete_from_measurement_by_id(
        tntdb::Connection &conn,
        m_msrmnt_id_t      id)
{
    LOG_START;
    db_reply_t ret = db_reply_new();
    try {
        tntdb::Statement st = conn.prepareCached(
                " DELETE FROM"
                "   t_bios_measurement"
                " WHERE id = :id"
                );

        ret.affected_rows = st.set("id", id)
                              .execute();
        ret.status = 1;
    } catch(const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
    LOG_END;
    return ret;
}

} //namespace persist
