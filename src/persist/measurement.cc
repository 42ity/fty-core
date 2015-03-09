#include <vector>
#include <tntdb/row.h>
#include <tntdb/result.h>
#include <tntdb/error.h>
#include <tntdb/statement.h>
#include <tntdb/transaction.h>

#include "log.h"
#include "defs.h"
#include "measurement.h"

namespace persist {

db_reply_t
insert_into_measurement(
        tntdb::Connection &conn,
        const char* topic,
        m_msrmnt_value_t value,
        m_msrmnt_scale_t scale,
        time_t time,
        const char* units) {

    LOG_START;
    db_reply_t ret = db_reply_new();

    if (! units ) {
        ret.errtype = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg = "NULL value of units not allowed";
        log_error("end: %s", ret.msg);
        return ret;
    }

    try {
        tntdb::Statement st = conn.prepareCached(
                "INSERT INTO t_bios_measurement_topic (topic, units) "
                "SELECT :topic, :units FROM t_empty WHERE NOT EXISTS "
                "(SELECT id from t_bios_measurement_topic WHERE topic=:topic AND "
                " units=:units)");
        uint32_t n = st.set("topic", topic)
                       .set("units", units)
                       .execute();

        log_debug("[t_bios_measurement_topic]: inserted %" PRIu32 " rows ", n);

        //XXX: time should be from DB only, discuss with Miska and Alenka
        //use UNIX_TIMESTAMP instead!!!
        st = conn.prepareCached(
                "INSERT INTO t_bios_measurement "
                "(timestamp, value, scale, units, topic_id) "
                "SELECT FROM UNIXTIME(:time), :value, :scale, :units, id FROM "
                "t_bios_measurement_topic WHERE topic=:topic AND units=:units");
        ret.affected_rows = st.set("topic", topic)
                              .set("time",  time)
                              .set("value", value)
                              .set("scale", scale)
                              .set("units", units)
                              .execute();

        log_debug("[t_bios_measurement]: inserted %" PRIu64 " rows ", ret.affected_rows);

        ret.status = 1;
        ret.rowid = conn.lastInsertId();
    } catch(const std::exception &e) {
        ret.errtype = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg = e.what();
        LOG_END_ABNORMAL(e);
    }
    LOG_END;

    return ret;
}

db_reply<std::vector<db_msrmnt_t>>
select_from_measurement_by_topic(
        tntdb::Connection &conn,
        const char* topic) {
    
    LOG_START;
    std::vector<db_msrmnt_t> item{};
    db_reply<std::vector<db_msrmnt_t>> ret = db_reply_new(item);

    if (! topic ) {
        ret.errtype = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg = "NULL value of topic is not allowed";
        log_error("end: %s", ret.msg);
        return ret;
    }
    
    try {
        tntdb::Statement st = conn.prepareCached(
                " SELECT :id, :timestamp, :value, :scale, :device_id, :units, :topic"
                " FROM v_bios_measurement"
                " WHERE topic LIKE '%:topic%'");
        ret.affected_rows = st.set("topic", topic)
                              .execute();

        for (tntdb::Statement::const_iterator it = st.begin();
                it != st.end(); ++it) {
            tntdb::Row r = *it;

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
    
    } catch(const tntdb::NotFound &e) {
        ret.errtype = DB_ERR;
        ret.errsubtype = DB_ERROR_NOTFOUND;
        ret.msg = e.what();
        LOG_END_ABNORMAL(e);
    } catch(const std::exception &e) {
        ret.errtype = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg = e.what();
        LOG_END_ABNORMAL(e);
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
        //TODO: is this mandatory?
        if (ret.affected_rows == 0) {
            ret.errtype = DB_ERR;
            ret.errsubtype = DB_ERROR_NOTFOUND;
            ret.msg = "Not found";
            log_debug("[t_bios_measurement]: id %" PRIu64 " not found\n", id);
            return ret;
        }

    } catch(const std::exception &e) {
        ret.errtype = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg = e.what();
        LOG_END_ABNORMAL(e);
    }

    LOG_END;
    return ret;
}

} //namespace persist
