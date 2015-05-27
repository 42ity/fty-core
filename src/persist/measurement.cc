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
    LOG_START;
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
        );
        uint32_t n = st.set("topic", topic)
                       .set("units", units)
                       .set("name", device_name)
                       .execute();

        log_debug("[t_bios_measurement_topic]: inserted %" PRIu32 " rows ", n);

        st = conn.prepareCached(
                " INSERT INTO"
                "   t_bios_measurement"
                "       (timestamp, value, scale, topic_id)"
                " SELECT"
                "   FROM_UNIXTIME(:time), :value, :scale, id"
                " FROM"
                "   t_bios_measurement_topic"
                " WHERE topic=:topic AND"
                "       units=:units"
        );
        log_debug("[t_bios_measurement]: inserting %" PRIi32 " * 10^%" PRIi16 " %s", value, scale, units);
        ret.affected_rows = st.set("topic", topic)
                              .set("time",  time)
                              .set("value", value)
                              .set("scale", scale)
                              .set("units", units)
                              .execute();

        log_debug("[t_bios_measurement]: inserted %" PRIu64 " rows ", ret.affected_rows);

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
            log_debug("[t_bios_measurement_topic]: new discovered device '%s' inserted %" PRIu32 " rows ",
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
                log_debug("[t_bios_measurement_topic]: t_bios_monitor_asset_relation inserted %" PRIu32 " rows ", n);
                goto insert_into_measurement_again; // successfully inserted into _discovered_device, save measurement one more time
            }
        }
        
        ret.rowid = conn.lastInsertId();
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
                " SELECT id, UNIX_TIMESTAMP(timestamp), value, scale, device_id, units, topic"
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

void get_measurements(ymsg_t* out, char** out_subj,
                      ymsg_t* in, const char* in_subj) {

    (*out_subj) = strdup("return_measurements");
    std::string json;
    try {
       std::string query = std::string(
            " SELECT topic, value, scale, UNIX_TIMESTAMP(timestamp), units "
            " FROM v_bios_measurement"
            " WHERE "
            " topic_id IN "
            "   (SELECT t.id FROM t_bios_measurement_topic AS t, "
            "                     t_bios_monitor_asset_relation AS rel "
            "    WHERE rel.id_asset_element = :id AND "
            "          t.device_id = id_discovered_device AND "
            "          t.topic LIKE :topic) AND "
            " timestamp ") +
            std::string((in_subj && strlen(in_subj) && in_subj[strlen(in_subj)-1] == '>') ? ">=" : ">") +
            " FROM_UNIXTIME(:time_st) AND "
            " timestamp "  +
            std::string((in_subj && strlen(in_subj) && in_subj[strlen(in_subj)-2] == '<') ? "<=" : "<") +
            " FROM_UNIXTIME(:time_end) "
            " ORDER BY timestamp ASC";

        int64_t start_ts = -1, end_ts = -1, last_ts = -1;
        uint64_t element_id = 0;
        _scoped_char *source = NULL;

        int rv = bios_db_measurements_read_request_extract (in, &start_ts, &end_ts, &element_id, &source);
        if (rv != 0) {
            throw std::invalid_argument ("bios_db_measurements_read_request_extract failed.");
        }

        std::string topic (source);
        FREE0 (source)
        topic += "@%";
        log_debug("Got request regarding '%s' from %" PRId64 " till %" PRId64" for %" PRId64, topic.c_str(), start_ts, end_ts, element_id);

        tntdb::Connection conn = tntdb::connectCached(url);
        {
            tntdb::Statement st = conn.prepareCached(
            "SELECT COUNT(t.id) FROM t_bios_measurement_topic AS t, "
            "                        t_bios_monitor_asset_relation AS rel "
            "    WHERE rel.id_asset_element = :id AND "
            "          t.device_id = id_discovered_device AND "
            "          t.topic LIKE :topic");
            tntdb::Row res = st.set("id", element_id).set("topic", topic).selectRow();
            if(res[0].getInt32() == 0)
                throw std::invalid_argument ("Invalid device or topic");
        }
 
        log_debug("Running query '%s'\n", query.c_str());
        tntdb::Statement st = conn.prepareCached(query);
        st.set("id", element_id).set("topic", topic).set("time_st", start_ts).set("time_end", end_ts);

        tntdb::Result result = st.select();

        std::string units;

        for(auto &row: result) {
            if(!json.empty()) {
                json += ",\n";
            }
            if(units.empty())
                units = row[4].getString();
            json += " {";
            json += "   \"value\": " + std::to_string(row[1].getInt32 ()) +  ",";
            json += "   \"scale\": " + std::to_string(row[2].getInt32()) +  ",";
            json += "   \"timestamp\": " + std::to_string(row[3].getInt64());
            json += " }";
            last_ts = row[3].getInt64();
        }

        json = "{ \"unit\": \"" + units + "\",\n" +
               "  \"source\": \"" + ymsg_get_string(in,"source") + "\",\n" +
               "  \"element_id\": " + ymsg_get_string(in,"element_id") + ",\n" +
               "  \"start_ts\": " + ymsg_get_string(in,"start_ts") + ",\n" +
               "  \"end_ts\": " + ymsg_get_string(in,"end_ts") + ",\n" +
               "  \"last_ts\": " + std::to_string(last_ts) + ",\n" +
               "\"data\": [\n" + json + "\n] }";

        _scoped_zchunk_t *ch = zchunk_new (json.c_str (), json.length ());
        if (!ch) {
            throw std::invalid_argument ("zchunk_new failed.");
        }
        ymsg_set_response (out, &ch);
        ymsg_set_status (out, true);

        log_debug ("json:\n%s", json.c_str ());
       
    } catch(const std::exception &e) {
        log_error("Ran into '%s' while getting measurements", e.what());
        ymsg_set_status (out, false);
        ymsg_set_errmsg (out, e.what());
        return;
    } catch (...) {
        log_error("unknown exception caught");
        ymsg_set_status (out, false);
        return;
    }
}

} //namespace persist
