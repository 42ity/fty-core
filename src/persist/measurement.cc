#include <tntdb/row.h>
#include <tntdb/result.h>
#include <tntdb/error.h>
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
    db_reply_t ret {0, 0, 0, NULL, NULL, NULL, 0, 0};

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

} //namespace persist
