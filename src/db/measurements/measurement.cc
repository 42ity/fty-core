#include <tntdb.h>

#include "dbpath.h"
#include "log.h"
#include "cleanup.h"

#include "measurement.h"

// Note: The individual sql queries being executed can perhaps be later replaced by measurements_basic operations

namespace persist {

reply
get_measurements
(uint64_t element_id, const char *topic, int64_t start_timestamp, int64_t end_timestamp, bool left_interval_closed, bool right_interval_closed,
 std::map <int64_t, double>& measurements, std::string& unit) {
    assert (topic);
    measurements.clear ();
    reply ret;
    log_debug ("Interval: %s%" PRId64", %" PRId64"%s Element id: %" PRIu64" Topic: %s",
               left_interval_closed ? "<" : "(", start_timestamp, end_timestamp,
               right_interval_closed ? ">" : ")", element_id, topic);
    
    try {        
        tntdb::Connection connection = tntdb::connectCached (url);
        // Find out if element_id exists
        {
            tntdb::Statement statement = connection.prepareCached ("SELECT * FROM t_bios_asset_element WHERE id_asset_element = :id");
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

            tntdb::Statement statement = connection.prepareCached (
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
            tntdb::Statement statement = connection.prepareCached (query);
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

reply
get_measurements_averages
(uint64_t element_id, const char *topic, const char *step, int64_t start_timestamp, int64_t end_timestamp, std::map <int64_t, double>& measurements, std::string& unit, int64_t& last_timestamp) {
    assert (topic);
    assert (step);
    // TODO: check step
    std::string averages_topic (topic);
    averages_topic.append ("_").append (step);

    try {        
        tntdb::Connection connection = tntdb::connectCached (url);

        tntdb::Statement statement = connection.prepareCached (
        " SELECT UNIX_TIMESTAMP(timestamp) FROM v_bios_measurement "
        " WHERE topic_id = ( "
        "   SELECT t.id FROM t_bios_measurement_topic AS t, "
        "   t_bios_monitor_asset_relation AS rel "
        "   WHERE rel.id_asset_element = :id AND "
        "         t.device_id = id_discovered_device AND "
        "         t.topic LIKE :topic "
        "   ) "
        " ORDER BY timestamp DESC LIMIT 1 ");
        std::string query_topic;
        query_topic.append (averages_topic).append ("@%");
        tntdb::Result result = statement.set ("id", element_id).set ("topic", query_topic).select ();
        if (result.size () == 0) {
            last_timestamp = INT64_MIN;
        } else {
            result.getRow (0).getValue (0).get (last_timestamp);
        }
    }
    catch (const std::exception &e) {
        log_error("Exception caught: %s", e.what());
        reply ret; ret.rv = -1;
        return ret;
    }
    catch (...) {
        log_error("Unknown exception caught!");
        reply ret; ret.rv = -1;
        return ret;    
    }
    return get_measurements (element_id, averages_topic.c_str (), start_timestamp, end_timestamp, true, true, measurements, unit);
}

reply
get_measurements_sampled
(uint64_t element_id, const char *topic, int64_t start_timestamp, int64_t end_timestamp, std::map <int64_t, double>& measurements, std::string& unit) {
    assert (topic);
    return get_measurements (element_id, topic, start_timestamp, end_timestamp, true, true, measurements, unit);
}


} // namespace persist

