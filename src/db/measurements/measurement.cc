#include <tntdb.h>

#include "log.h"
#include "cleanup.h"
#include "utils.h"
#include "dbtypes.h"

#include "measurement.h"

// Note: The individual sql queries being executed can perhaps be later replaced by measurements_basic operations

namespace persist {

reply_t
get_measurements (
        tntdb::Connection &conn,
        uint64_t element_id,
        const char *topic,
        int64_t start_timestamp,
        int64_t end_timestamp,
        bool left_interval_closed,
        bool right_interval_closed,
        std::map <int64_t, double>& measurements,
        std::string& unit)
{
    assert (topic);
    measurements.clear ();
    reply_t ret;
    log_debug ("Interval: %s%" PRId64", %" PRId64"%s Element id: %" PRIu64" Topic: %s",
               left_interval_closed ? "<" : "(", start_timestamp, end_timestamp,
               right_interval_closed ? ">" : ")", element_id, topic);
    
    try {        
        // Find out if element_id exists
        {
            tntdb::Statement statement = conn.prepareCached ("SELECT * FROM t_bios_asset_element WHERE id_asset_element = :id");
            tntdb::Result result = statement.set ("id", element_id).select();
            if (result.size () != 1) {
                log_debug ("element id: %" PRIu64" does NOT exists", element_id);
                ret.rv = 1;
                return ret;
            }
            log_debug ("element id: %" PRIu64" exists", element_id);
        }
        // Element_id exists, find out if there is an entry in t_bios_measurement_topic         
        uint64_t topic_id = 0;
        {
            std::string query_topic (topic);
            query_topic += "@%";

            tntdb::Statement statement = conn.prepareCached (
            "SELECT t.id, t.units FROM t_bios_measurement_topic AS t, "
            "                          t_bios_monitor_asset_relation AS rel "
            "    WHERE rel.id_asset_element = :id AND "
            "          t.device_id = id_discovered_device AND "
            "          t.topic LIKE :topic");
            tntdb::Result result = statement.set ("id", element_id).set ("topic", query_topic).select ();
            if (result.size () != 1) {
                ret.rv = 2;
                return ret;
            }
            result.getRow (0).getValue (0).get (topic_id);
            result.getRow (0).getValue (1).get (unit);
            log_debug ("topic id: %" PRIu64" unit: %s", topic_id, unit.c_str ());
        }
        // Get measurements  
        {
            std::string query (
            " SELECT topic, value, scale, UNIX_TIMESTAMP(timestamp) "
            " FROM v_bios_measurement"
            " WHERE "
            " topic_id = :topic_id AND "
            " timestamp ");
            query.append (left_interval_closed ? ">=" : ">")
                 .append (" FROM_UNIXTIME(:time_st) AND timestamp ")
                 .append (right_interval_closed ? "<=" : "<")
                 .append (" FROM_UNIXTIME(:time_end) ")
                 .append (" ORDER BY timestamp ASC");

            log_debug("Running query: '%s'", query.c_str ());
            tntdb::Statement statement = conn.prepareCached (query);
            statement.set ("id", element_id).set ("topic_id", topic_id).set ("time_st", start_timestamp).set ("time_end", end_timestamp);

            tntdb::Result result = statement.select ();

            for (auto &row: result) {
                double comp_value = row[1].getInt32 () * std::pow (10, row[2].getInt32());
                measurements.emplace (std::make_pair (row[3].getInt64(), comp_value));                 
            }
        }
    }
    catch (const std::exception &e) {
        log_error("Exception caught: %s", e.what());
        ret.rv = -1;
        return ret;
    }
    catch (...) {
        log_error("Unknown exception caught!");
        ret.rv = -1;
        return ret;    
    }
    ret.rv = 0;
    return ret;
}

reply_t
get_measurements_averages (
        tntdb::Connection &conn,
        uint64_t element_id,
        const char *source,
        const char *type,
        const char *step,
        int64_t start_timestamp,
        int64_t end_timestamp,
        std::map <int64_t, double>& measurements,
        std::string& unit,
        int64_t& last_timestamp)
{
    assert (source);
    assert (type);
    assert (step);
    assert (is_average_step_supported (step));
    assert (is_average_type_supported (type));

    reply_t ret;
    std::string topic;    
    topic.assign (source).append ("_").append (type).append ("_").append (step);

    try {        
        uint64_t topic_id = 0;
        {
            std::string query_topic (topic);
            query_topic.append ("@%");

            tntdb::Statement statement = conn.prepareCached (
            " SELECT t.id FROM t_bios_measurement_topic AS t, "
            "            t_bios_monitor_asset_relation AS rel "
            "   WHERE rel.id_asset_element = :id AND "
            "         t.device_id = id_discovered_device AND "
            "         t.topic LIKE :topic ");

            tntdb::Result result = statement.set ("id", element_id).set ("topic", query_topic).select ();
            if (result.size () == 1) {
                result.getRow (0).getValue (0).get (topic_id);
            }
            else if (result.size () == 0) {
                last_timestamp = INT64_MIN;
                ret.rv = 2;
                return ret;
            }
            else {
                log_critical ("Query returned %d rows.", result.size ());
                ret.rv = -1;
                return ret;
            }
        }
        log_debug ("topic id = %" PRId64, topic_id);
        {
            tntdb::Statement statement = conn.prepareCached (
            " SELECT UNIX_TIMESTAMP(MAX(timestamp)) FROM v_bios_measurement "
            " WHERE topic_id = :topic_id ");
            tntdb::Result result = statement.set ("topic_id", topic_id).select ();
            if (result.size () == 0) {
                last_timestamp = INT64_MIN;
            }
            else if (result.size () == 1) {            
                result.getRow (0).getValue (0).get (last_timestamp);
            }
            else {
                log_critical ("Query returned %d rows.", result.size ());
                ret.rv = -1;
                return ret;
            }
        }
        log_debug ("last timestamp: '%" PRId64"'", last_timestamp);

    }
    catch (const std::exception &e) {
        log_error("Exception caught: %s", e.what());
        ret.rv = -1;
        return ret;
    }
    catch (...) {
        log_error("Unknown exception caught!");
        ret.rv = -1;
        return ret;    
    }
    return get_measurements (conn, element_id, topic.c_str (), start_timestamp, end_timestamp, true, true, measurements, unit);
}

reply_t
get_measurements_sampled (
        tntdb::Connection &conn,
        uint64_t element_id,
        const char *topic,
        int64_t start_timestamp,
        int64_t end_timestamp,
        std::map <int64_t, double>& measurements,
        std::string& unit)
{
    assert (topic);
    return get_measurements (conn, element_id, topic, start_timestamp, end_timestamp, true, true, measurements, unit);
}

reply_t
get_device_name_from_element_id (
        tntdb::Connection &conn,
        uint64_t element_id,
        std::string& device_name)
{
    reply_t ret;
    try {
        tntdb::Statement statement = conn.prepareCached (
        " SELECT a.name FROM "
        "    t_bios_discovered_device AS a LEFT JOIN t_bios_monitor_asset_relation AS b "
        "    on a.id_discovered_device = b.id_discovered_device "
        " WHERE id_asset_element = :element_id");
        tntdb::Result result = statement.set ("element_id", element_id).select();
        if (result.size () != 1) {
            ret.rv = -1;
            return ret;
        }
        result.getRow (0).getValue (0).get (device_name);
    }
    catch (const std::exception &e) {
        log_error("Exception caught: %s", e.what());
        ret.rv = -1;
        return ret;
    }
    catch (...) {
        log_error("Unknown exception caught!");
        ret.rv = -1;
        return ret;
    }
    ret.rv = 0;
    return ret;
}

reply_t
select_measurement_last_web_byElementId (
        tntdb::Connection &conn,
        const std::string& src,
        a_elmnt_id_t id,
        m_msrmnt_value_t& value,
        m_msrmnt_scale_t& scale)
{
    LOG_START;
    log_debug("id = %" PRIu32, id);

    std::string name;
    auto rep = get_device_name_from_element_id(conn, id, name);
    if (rep.rv != 0)
        return rep;

    std::string topic = src + "@" + name;
    reply_t ret;
    try {
        tntdb::Statement statement = conn.prepareCached (
        " SELECT v.value, v.scale FROM"
        "    v_bios_measurement_last v"
        " WHERE topic=:topic"
        );

        tntdb::Row row = statement.set ("topic", topic).selectRow();
        log_debug("[v_bios_measurement]: were selected %" PRIu32 " rows", 1);

        row[0].get(value);
        row[1].get(scale);
    }
    catch (const std::exception &e) {
        ret.rv = -1;
        LOG_END_ABNORMAL(e);
        return ret;
    }
    catch (...) {
        log_error("Unknown exception caught!");
        ret.rv = -1;
        return ret;
    }
    ret.rv = 0;
    LOG_END;
    return ret;
}

} // namespace persist

