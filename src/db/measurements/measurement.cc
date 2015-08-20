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
 * \author Karol Hrdina
 * \author Alena Chernikava
 * \author Michal Vyskocil
 * \author Michal Hrusecky
 * \brief Not yet documented file
 */
#include <tntdb.h>

#include "log.h"
#include "cleanup.h"
#include "utils.h"
#include "dbtypes.h"
#include "defs.h"

#include "measurement.h"

// Note: The individual sql queries being executed can perhaps be later replaced by measurements_basic operations

namespace persist {

reply_t
select_measurements (
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
                log_debug ("topic '%s' was found %u times", query_topic.c_str(), result.size());
                return ret;
            }
            result.getRow (0).getValue (0).get (topic_id);
            result.getRow (0).getValue (1).get (unit);
            log_debug ("topic id: %" PRIu64" unit: %s", topic_id, unit.c_str ());
        }
        // Get measurements  
        {
            std::string query (
            " SELECT topic, value, scale, timestamp "
            " FROM v_bios_measurement"
            " WHERE "
            " topic_id = :topic_id AND "
            " timestamp ");
            query.append (left_interval_closed ? ">=" : ">")
                 .append (" :time_st AND timestamp ")
                 .append (right_interval_closed ? "<=" : "<")
                 .append (" :time_end ")
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
select_measurements_averages (
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

    reply_t ret;
    std::string topic;    
    topic.assign (source).append ("_").append (type).append ("_").append (step);

    try {        
        uint64_t topic_id = 0;
        {
            std::string query_topic (topic);
            query_topic.append ("@%");

            // find topic id
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
                last_timestamp = INT32_MIN;
                ret.rv = 2;
                return ret;
            }
            else {
                log_critical ("Query returned '%d' rows. Exactly one row was expected!", result.size ());
                ret.rv = -1;
                return ret;
            }
        }
        log_debug ("topic id: '%" PRId64"'", topic_id);
        {
            tntdb::Statement statement = conn.prepareCached (
            " SELECT COALESCE(MAX(timestamp), :cval) FROM v_bios_measurement "
            " WHERE topic_id = :topic_id ");
            tntdb::Result result = statement.set ("topic_id", topic_id).set ("cval", INT32_MIN).select ();
            if (result.size () == 0) {
                log_debug ("result size: '0'. Assigning 'INT32_MIN' as timestamp of last average.");                
                last_timestamp = INT32_MIN;
            }
            else if (result.size () == 1) {            
                log_debug ("result size: 1");
                result.getRow (0).getValue (0).get (last_timestamp);
            }
            else {
                log_critical ("Query returned '%d' rows. Exactly one row was expected!", result.size ());
                ret.rv = -1;
                return ret;
            }
        }
        log_debug ("Timestamp of the last average: '%" PRId64"'", last_timestamp);

    }
    catch (const std::exception &e) {
        log_error("Exception caught: '%s'.", e.what ());
        ret.rv = -1;
        return ret;
    }
    catch (...) {
        log_error("Unknown exception caught.");
        ret.rv = -1;
        return ret;    
    }
    return select_measurements (conn, element_id, topic.c_str (), start_timestamp, end_timestamp, true, true, measurements, unit);
}

reply_t
select_measurements_sampled (
        tntdb::Connection &conn,
        uint64_t element_id,
        const char *topic,
        int64_t start_timestamp,
        int64_t end_timestamp,
        std::map <int64_t, double>& measurements,
        std::string& unit)
{
    assert (topic);
    return select_measurements (conn, element_id, topic, start_timestamp, end_timestamp, true, true, measurements, unit);
}

reply_t
select_device_name_from_element_id (
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
        m_msrmnt_scale_t& scale,
        int minutes_back)
{
    LOG_START;
    log_debug("id = %" PRIu32, id);

    std::string name;
    auto rep = select_device_name_from_element_id(conn, id, name);
    if (rep.rv != 0)
        return rep;

    std::string topic = src + "@" + name;
    LOG_END;
    return select_measurement_last_web_byTopic(conn, topic, value, scale, minutes_back, false);
}

reply_t
select_measurement_last_web_byTopicLike (
        tntdb::Connection &conn,
        const std::string& topic,
        m_msrmnt_value_t& value,
        m_msrmnt_scale_t& scale,
        int minutes_back) {
    return select_measurement_last_web_byTopic(conn, topic, value, scale, minutes_back, true);
}


reply_t
select_measurement_last_web_byTopic (
        tntdb::Connection &conn,
        const std::string& topic,
        m_msrmnt_value_t& value,
        m_msrmnt_scale_t& scale,
        int minutes_back,
        bool fuzzy)
{
    reply_t ret;

    std::string view;
    if ( minutes_back <= 10 )
        view = "v_web_measurement_10m";
    // TODO fix longer time period after demo
    else // if ( minutes_back <= 60*24 )
        view = "v_web_measurement_24h";
    log_debug ("fuzzy %s", fuzzy ? " true " : " false ");
    int64_t max_time = 0 ;
    try {
        std::string query_max_time =
        " SELECT max(timestamp) "
        " FROM " +
            view + " v " +
        " WHERE " +
        "   v.topic ";
        query_max_time += fuzzy ? " LIKE " : " = ";
        query_max_time += " :topic ";
        log_debug("%s", query_max_time.c_str());

        tntdb::Statement statement = conn.prepareCached(query_max_time);
        tntdb::Row row = statement.set("topic", topic).
                                   selectRow();

        log_debug("[%s]: were selected %" PRIu32 " rows," \
                  " topic %s '%s', selecting maxtime", 
                  view.c_str(), 1, fuzzy ? " LIKE " : " = ", topic.c_str());
        row[0].get(max_time);
        log_debug ( "maxtime = %" PRIi64 , max_time);
    }
    catch (const tntdb::NotFound &e) {
        ret.rv = 1;
        log_debug ("maxtime was not found (there is no "\
                   "measurement records) with topic %s '%s'", 
                   fuzzy ? " LIKE " : " = ", topic.c_str());
        return ret;
    }
    catch (const std::exception &e) {
        ret.rv = -1;
        log_debug("maxtime: topic %s '%s'", fuzzy ? " LIKE " : " = ", topic.c_str());
        LOG_END_ABNORMAL(e);
        return ret;
    }
    catch (...) {
        log_error("maxtime: Unknown exception caught!");
        ret.rv = -2;
        return ret;
    }

    try{
        std::string query =
        " SELECT value, scale FROM " +
            view +
        " WHERE topic ";
        query += fuzzy ? " LIKE " : " = ";
        query += " :topic AND ";
        query += " timestamp = :maxtime ";
        log_debug("%s", query.c_str());

        tntdb::Statement statement = conn.prepareCached(query);
        tntdb::Result res = statement.set("topic", topic).
                                      set("maxtime", max_time).
                                      select();
        log_debug("[%s]: were selected %u rows,"\
                  " topic %s '%s'", view.c_str(), res.size(), 
                  fuzzy ? " LIKE " : " = ", topic.c_str());
        switch ( res.size() )
        {
            case 0:
            {
                log_error (" This should never happen: "\
                           " row with max %" PRIi64 " timestamp " \
                           " and topic %s '%s'" \
                           " disappeared", max_time, fuzzy ? " LIKE " : " = ", topic.c_str());
                ret.rv = 2;
                break;
            }
            case 1:
            {
                tntdb::Row row = *res.begin();
                row[0].get(value);
                row[1].get(scale);
                ret.rv = 0;
                break;
            }
            default:
            {
                ret.rv = 3;
                log_error (" Unexpected number of rows returned: " \
                           " (timestamp, topic_id) not unique"
                           " timestamp = %" PRIi64 " and topic %s '%s'", 
                           max_time, fuzzy ? " LIKE " : " = ", topic.c_str());
                break;
            }
        }
        return ret;
    }
    catch (const std::exception &e) {
        ret.rv = -1;
        log_debug("valuescale: topic = %s", topic.c_str());
        LOG_END_ABNORMAL(e);
        return ret;
    }
    catch (...) {
        log_error("valuescale: Unknown exception caught!");
        ret.rv = -2;
        return ret;
    }
}

int
    delete_measurements(
        tntdb::Connection &conn,
        m_msrmnt_tp_id_t   topic_id,
        m_msrmnt_id_t     &affected_rows
        )
{
    LOG_START;
    log_debug ("  topic_id = %" PRIu16, topic_id);

    try{
        tntdb::Statement st = conn.prepareCached(
            " DELETE"
            " FROM"
            "   t_bios_measurement"
            " WHERE"
            "   topic_id = :topicid"
        );

        affected_rows = st.set("topicid", topic_id).
                           execute();
        log_debug ("[t_bios_measurement]: was deleted %"
                                PRIu64 " rows", affected_rows);
        LOG_END;
        return 0;
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return 1;
    }
}

int
    delete_measurement_topic(
        tntdb::Connection &conn,
        m_msrmnt_tp_id_t   topic_id,
        m_msrmnt_tp_id_t  &affected_rows
        )
{
    LOG_START;
    log_debug ("  topic_id = %" PRIu16, topic_id);

    try{
        tntdb::Statement st = conn.prepareCached(
            " DELETE"
            " FROM"
            "   t_bios_measurement_topic"
            " WHERE"
            "   id = :topicid"
        );

        affected_rows = st.set("topicid", topic_id).
                           execute();
        log_debug ("[t_bios_measurement_topic]: was deleted %"
                                PRIu16 " rows", affected_rows);
        LOG_END;
        return 0;
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return 1;
    }
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


int
    select_for_element_topics_all(
            tntdb::Connection& conn,
            a_elmnt_id_t element_id,
            std::function<void(
                const tntdb::Row&
                )>& cb)
{
    LOG_START;

    try{
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "   v.id"
            " FROM "
            "   t_bios_measurement_topic t "
            " JOIN "
            "   t_bios_monitor_asset_relation t1 "
            " ON "
            "   ( t.device_id = t1.id_discovered_device ) AND "
            "     t1.id_asset_element = :idelement "
        );

        tntdb::Result res = st.set("idelement", element_id).
                               select();

        for (const auto& r: res) {
            cb(r);
        }
        LOG_END;
        return 0;
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return -1;
    }
}


} // namespace persist

