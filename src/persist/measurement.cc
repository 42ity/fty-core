#include <vector>
#include <tntdb/result.h>
#include <tntdb/statement.h>

#include "log.h"
#include "measurement.h"

namespace persist {

db_reply_t
    insert_into_measurement(
        tntdb::Connection &conn,
        const char        *topic,
        m_msrmnt_value_t   value,
        m_msrmnt_scale_t   scale,
        time_t             time,
        const char        *units,
        const char        *device_name)
{
    LOG_START;
    db_reply_t ret = db_reply_new();

    if ( !units ) {
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "NULL value of units is not allowed";
        log_error("end: %s", ret.msg);
        return ret;
    }
    
    if ( !topic ) {
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "NULL value of topic is not allowed";
        log_error("end: %s", ret.msg);
        return ret;
    }

    try {
        tntdb::Statement st = conn.prepareCached(
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

                /*
                " WHERE NOT EXISTS"
                "  (SELECT"
                "       id"
                "   FROM"
                "       t_bios_measurement_topic"
                "   WHERE topic=:topic AND"
                "         units=:units)"*/
        );
        uint32_t n = st.set("topic", topic)
                       .set("units", units)
                       .set("name", device_name)
                       .execute();

        log_debug("[t_bios_measurement_topic]: inserted %" PRIu32 " rows ", n);

        st = conn.prepareCached(
                " INSERT INTO t_bios_measurement"
                " (timestamp, value, scale, topic_id)"
                " SELECT FROM_UNIXTIME(:time), :value, :scale, id FROM"
                " t_bios_measurement_topic WHERE topic=:topic AND units=:units");
        log_debug("[t_bios_measurement]: inserting %" PRIi32 " * 10^%" PRIi16 " %s", value, scale, units);
        ret.affected_rows = st.set("topic", topic)
                              .set("time",  time)
                              .set("value", value)
                              .set("scale", scale)
                              .set("units", units)
                              .execute();

        log_debug("[t_bios_measurement]: inserted %" PRIu64 " rows ", ret.affected_rows);

        ret.rowid = conn.lastInsertId();
        ret.status = 1;
    } catch(const std::exception &e) {
        ret.status = 0;
        ret.errtype = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }

    LOG_END;
    return ret;
}

db_reply <std::vector<db_msrmnt_t>>
    select_from_measurement_by_topic(
        tntdb::Connection &conn,
        const char* topic)
{   
    LOG_START;
    std::vector<db_msrmnt_t> item{};
    db_reply<std::vector<db_msrmnt_t>> ret = db_reply_new(item);

    if ( !topic ) {
        ret.errtype = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg = "NULL value of topic is not allowed";
        log_error("end: %s", ret.msg);
        return ret;
    }
    
    try {
        tntdb::Statement st = conn.prepareCached(
                " SELECT id, UNIX_TIMESTAMP(timestamp), value, scale, device_id, units, topic"
                " FROM v_bios_measurement"
                " WHERE topic LIKE :topic");
        tntdb::Result res = st.set("topic", topic)
                              .select();
        
        log_debug ("was %u rows selected", res.size());

        for (auto &r : res ) {

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
        ret.status = 0;
        ret.errtype = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg = e.what();
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
        m_msrmnt_id_t id) {
    LOG_START;
    db_reply_t ret = db_reply_new();
    try {
        tntdb::Statement st = conn.prepareCached(
                " DELETE FROM"
                " t_bios_measurement "
                " WHERE id = :id"
                );

        ret.affected_rows = st.set("id", id)
                              .execute();
        ret.status = 1;
    } catch(const std::exception &e) {
        ret.status = 0;
        ret.errtype = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }

    LOG_END;
    return ret;
}

} //namespace persist
