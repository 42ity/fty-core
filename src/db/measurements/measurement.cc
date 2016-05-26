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
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \author Michal Vyskocil <MichalVyskocil@Eaton.com>
 * \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 * \brief Not yet documented file
 */
#include <tntdb.h>
#include <cxxtools/join.h>

#include "log.h"
#include "defs.h"

#include "measurement.h"

// Note: The individual sql queries being executed can perhaps be later replaced by measurements_basic operations

namespace persist {
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

// generate the proper tntdb::Statement for multi value delete for measurement topics
static tntdb::Statement
    s_multi_delete_topics_statement(
        tntdb::Connection& conn,
        std::set<m_msrmnt_tpc_id_t> topics)
{
    static const std::string sql_header = "DELETE FROM t_bios_measurement_topic where id in (";
    const std::string sql_body = cxxtools::join(topics.cbegin(), topics.cend(), ", ");
    static const std::string sql_end = ")";

    log_debug("sql: '%s'", (sql_header + sql_body + sql_end).c_str());
    
    auto st = conn.prepare(sql_header + sql_body + sql_end);

    return st;
}

// generate the proper tntdb::Statement for multi value delete for measurements
static tntdb::Statement
    s_multi_delete_measurements_statement(
        tntdb::Connection& conn,
        std::set<m_msrmnt_tpc_id_t> topics)
{
    static const std::string sql_header = "DELETE FROM t_bios_measurement where topic_id in (";
    const std::string sql_body = cxxtools::join(topics.cbegin(), topics.cend(), ", ");
    static const std::string sql_end = ")";

    log_debug("sql: '%s'", (sql_header + sql_body + sql_end).c_str());
    
    auto st = conn.prepare(sql_header + sql_body + sql_end);

    return st;
}

int
    delete_measurements(
        tntdb::Connection& conn,
        std::set<m_msrmnt_tpc_id_t> topics)
{
    LOG_START;
    if ( topics.empty() ) {
        return 0;
    }
    try {
        auto st = s_multi_delete_measurements_statement(conn, topics);
        st.execute();
        LOG_END;
        return 0;
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return 1;
    }
}


int
    delete_measurement_topics(
        tntdb::Connection& conn,
        std::set<m_msrmnt_tpc_id_t> topics)
{
    LOG_START;
    if ( topics.empty() ) {
        return 0;
    }
    try {
        auto st = s_multi_delete_topics_statement(conn, topics);
        st.execute();
        LOG_END;
        return 0;
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return 1;
    }
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
            "   t.id"
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
