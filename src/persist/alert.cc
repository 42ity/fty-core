/*
Copyright (C) 2014-2015 Eaton
 
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*! \file alert.cc
    \brief Pure DB API for CRUD operations on alerts

    \author Alena Chernikava <alenachernikava@eaton.com>
*/

#include <tntdb/row.h>
#include <tntdb/result.h>
#include <tntdb/error.h>
#include <tntdb/transaction.h>
#include <cxxtools/split.h>

#include "log.h"
#include "defs.h"
#include "alert.h"
#include "agents.h"
#include "dbpath.h"
#include "bios_agent.h"

namespace persist {

//=============================================================================
// end date of the alert can't be specified during the insert statement
db_reply_t
    insert_into_alert 
        (tntdb::Connection  &conn,
         const char         *rule_name,
         a_elmnt_pr_t        priority,
         m_alrt_state_t      alert_state,
         const char         *description,
         m_alrt_ntfctn_t     notification,
         int64_t             date_from)
{
    LOG_START;
    log_debug ("  rule_name = '%s'", rule_name);
    log_debug ("  priority = %" PRIu16, priority);
    log_debug ("  alert_state = %" PRIu16, alert_state);
    log_debug ("  description = '%s'", description);
    log_debug ("  date_from = %" PRIi64, date_from);
    log_debug ("  notification = %" PRIu16, notification);

    db_reply_t ret = db_reply_new();

    // input parameters control 
    if ( !is_ok_rule_name (rule_name) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "rule name is invalid";
        log_error ("end: %s, %s","ignore insert", ret.msg);
        return ret;
    }
    if ( !is_ok_priority (priority) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "unsupported value of priority";
        log_error ("end: %s, %s", "ignore insert", ret.msg);
        return ret;
    }
    if ( !is_ok_alert_state (alert_state) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "alert state is invalid";
        log_error ("end: %s, %s","ignore insert", ret.msg);
        return ret;
    }
    // description can be even NULL
    // notification can be any number. It is treated as a bit-vector
    // ATTENTION: no control for time
    log_debug ("input parameters are correct");
    
    try{
        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            "   t_bios_alert"
            "   (rule_name, priority, state, description,"
            "   notification, date_from)"
            " SELECT"
            "    :rule, :priority, :state, :desc, :note, FROM_UNIXTIME(:from)"
            " FROM"
            "   t_empty"
            " WHERE NOT EXISTS"
            "   ("
            "       SELECT"
            "           id"
            "       FROM"
            "           t_bios_alert v"
            "       WHERE"
            "           v.date_till is NULL AND"
            "           v.rule_name = :rule"
            "   )"
        );
   
        ret.affected_rows = st.set("rule", rule_name).
                               set("priority", priority).
                               set("state", alert_state).
                               set("desc", description).
                               set("note", notification).
                               set("from", date_from).
                               execute();
        ret.rowid = conn.lastInsertId();
        log_debug ("[t_bios_alert]: was inserted %" 
                                    PRIu64 " rows", ret.affected_rows);
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}


//=============================================================================

db_reply_t
    update_alert_notification_byId 
        (tntdb::Connection  &conn,
         m_alrt_ntfctn_t     notification,
         m_alrt_id_t         id)
{
    LOG_START;
    log_debug ("  notification = %" PRIu16, notification);
    log_debug ("  id = %" PRIu32, id);

    db_reply_t ret = db_reply_new();
    
    try{
        tntdb::Statement st = conn.prepareCached(
            " UPDATE"
            "   t_bios_alert"
            " SET notification = (notification | :note)"  // a bitwire OR
            " WHERE  id = :id"
        );
   
        ret.affected_rows = st.set("id", id).
                               set("note", notification).
                               execute();
        ret.rowid = conn.lastInsertId();
        log_debug ("[t_bios_alert]: was updated %" 
                                    PRIu64 " rows", ret.affected_rows);
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
db_reply_t
    update_alert_tilldate_by_rulename
        (tntdb::Connection  &conn,
         int64_t             date_till,
         const char         *rule_name)
{
    LOG_START;
    log_debug ("  tilldate = %" PRIi64, date_till);
    log_debug ("  rule_name = %s", rule_name);

    db_reply_t ret = db_reply_new();
    
    try{
        tntdb::Statement st = conn.prepareCached(
            " UPDATE"
            "   t_bios_alert"
            " SET date_till = FROM_UNIXTIME(:till)"
            " WHERE rule_name = :rule AND"
            "   date_till is NULL"
        );
   
        ret.affected_rows = st.set("rule", rule_name).
                               set("till", date_till).
                               execute();
        ret.rowid = conn.lastInsertId();
        log_debug ("[t_bios_alert]: was updated %" 
                                    PRIu64 " rows", ret.affected_rows);
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}


//=============================================================================
db_reply_t
    update_alert_tilldate 
        (tntdb::Connection  &conn,
         int64_t             date_till,
         m_alrt_id_t         id)
{
    LOG_START;
    log_debug ("  tilldate = %" PRIi64, date_till);
    log_debug ("  id = %" PRIu32, id);

    db_reply_t ret = db_reply_new();
    
    try{
        tntdb::Statement st = conn.prepareCached(
            " UPDATE"
            "   t_bios_alert"
            " SET date_till = FROM_UNIXTIME(:till)"
            " WHERE  id = :id"
        );
   
        ret.affected_rows = st.set("id", id).
                               set("till", date_till).
                               execute();
        ret.rowid = conn.lastInsertId();
        log_debug ("[t_bios_alert]: was updated %" 
                                    PRIu64 " rows", ret.affected_rows);
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
db_reply_t
    delete_from_alert
        (tntdb::Connection &conn,
         m_alrt_id_t id)
{
    LOG_START;
    log_debug ("  id = %" PRIu32, id);

    db_reply_t ret = db_reply_new();

    try{
        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            "   t_bios_alert"
            " WHERE"
            "   id = :id"
        );
   
        ret.affected_rows = st.set("id", id).
                               execute();
        ret.rowid = conn.lastInsertId();
        log_debug ("[t_bios_alert]: was deleted %" 
                                        PRIu64 " rows", ret.affected_rows);
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}


//=============================================================================
db_reply_t
    insert_into_alert_devices 
        (tntdb::Connection  &conn,
         m_alrt_id_t               alert_id,
         std::vector <std::string> device_names)
{
    LOG_START;
    log_debug ("  alert_id = %" PRIi32, alert_id);
    log_debug ("  devices count = %zu", device_names.size());

    db_reply_t ret = db_reply_new();

    // input parameters control
    if ( alert_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of alert_id is not allowed";
        log_error ("end: %s, %s", "ignore insert", ret.msg);
        return ret;
    }
    if ( device_names.size() == 0 )
    {
        ret.status  = 1;
        log_info ("end: %s, %s","ignore insert", "noting to insert");
        return ret;
    }
    // actually, if there is nothing to insert, then insert was ok :)
    log_debug ("input parameters are correct");

    ret.status = 1;
    for (auto &device_name : device_names )
    {
        auto reply_internal = insert_into_alert_device
                                (conn, alert_id, device_name.c_str());
        if ( ( reply_internal.status == 1 ) && ( reply_internal.affected_rows == 1 ) )
            ret.affected_rows++;
        ret.status = ret.status && reply_internal.status;
        if ( ret.errtype == 0 )
            ret.errtype = reply_internal.errtype;
        if ( ret.errsubtype == 0 )
            ret.errsubtype = reply_internal.errsubtype;
        if ( ret.msg == NULL )
            ret.msg = reply_internal.msg;
    }
    LOG_END;
    return ret;
}

db_reply_t
    insert_into_alert_device
        (tntdb::Connection &conn,
         m_alrt_id_t        alert_id,
         const char        *device_name)
{
    LOG_START;
    log_debug ("  alert_id = %" PRIi32, alert_id);
    log_debug ("  device_name = '%s'", device_name);

    db_reply_t ret = db_reply_new();

    // input parameters control
    if ( alert_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of alert_id is not allowed";
        log_error ("end: %s, %s", "ignore insert", ret.msg);
        return ret;
    }
    if ( !is_ok_name(device_name) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "value of device_name is invalid";
        log_error ("end: %s, %s", "ignore insert", ret.msg);
        return ret;
    }
    log_debug ("input parameters are correct");

    try{
        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO"
            "   t_bios_alert_device"
            "   (alert_id, device_id)"
            " SELECT"
            "   :alert, v.id_discovered_device"
            " FROM"
            "   t_bios_monitor_asset_relation v,"
            "   t_bios_asset_element v1"
            " WHERE"
            "   v.id_asset_element = v1.id_asset_element AND"
            "   v1.name = :name AND"
            "   NOT EXISTS"
            "   ("
            "       SELECT"
            "           1"
            "       FROM"
            "           t_bios_alert_device v2"
            "       WHERE"
            "           v2.alert_id = :alert AND"
            "           v2.device_id = v.id_discovered_device"
            "   )"
        );
   
        ret.affected_rows = st.set("alert", alert_id).
                               set("name", device_name).
                               execute();
        ret.rowid = conn.lastInsertId();
        log_debug ("[t_bios_alert_device]: was inserted %" 
                                    PRIu64 " rows", ret.affected_rows);
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
db_reply_t
    delete_from_alert_device
        (tntdb::Connection &conn,
         m_alrtdvc_id_t    id)
{
    LOG_START;
    log_debug ("  id = %" PRIu32, id);

    db_reply_t ret = db_reply_new();

    try{
        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            "   t_bios_alert_device"
            " WHERE"
            "   id = :id"
        );
   
        ret.affected_rows = st.set("id", id).
                               execute();
        log_debug ("[t_bios_alert_device]: was deleted %" 
                                        PRIu64 " rows", ret.affected_rows);
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
db_reply_t
    delete_from_alert_device_byalert
        (tntdb::Connection &conn,
         m_alrt_id_t         id)
{
    LOG_START;
    log_debug ("  id = %" PRIu32, id);

    db_reply_t ret = db_reply_new();

    try{
        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            "   t_bios_alert_device"
            " WHERE"
            "   alert_id = :id"
        );
   
        ret.affected_rows = st.set("id", id).
                               execute();
        log_debug ("[t_bios_alert_device]: was deleted %" 
                                        PRIu64 " rows", ret.affected_rows);
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
db_reply_t
    insert_new_alert 
        (tntdb::Connection  &conn,
         const char         *rule_name,
         a_elmnt_pr_t        priority,
         m_alrt_state_t      alert_state,
         const char         *description,
         m_alrt_ntfctn_t     notification,
         int64_t             date_from,
         std::vector<std::string> device_names)
{
    LOG_START;
 
    tntdb::Transaction trans(conn);
    auto reply_internal1 = insert_into_alert
        (conn, rule_name, priority, alert_state, description,
         notification, date_from);
    if ( ( reply_internal1.status == 0 ) ||
         ( reply_internal1.affected_rows == 0 ) )
    {
        trans.rollback();
        log_info ("end: alarm was not inserted (fail in alert");
        return reply_internal1;
    }
    auto alert_id = reply_internal1.rowid;

    auto reply_internal2 = insert_into_alert_devices
                                    (conn, alert_id, device_names);
    if ( ( reply_internal2.status == 0 ) ||
         ( reply_internal2.affected_rows == 0 ) )
    {
        trans.rollback();
        log_info ("end: alarm was not inserted (fail in alert devices");
        return reply_internal2;
    }
    trans.commit();
    LOG_END;
    return reply_internal1;
}


//=============================================================================

//
// TODO: LIMITS - those queries can be potentially HUGE, but our db does not support the queries
//       with IN and sub select
// MariaDB [box_utf8]> SELECT * FROM v_bios_alert_all v WHERE v.id IN (SELECT id FROM v_bios_alert ORDER BY id LIMIT 30);
// ERROR 1235 (42000): This version of MariaDB doesn't yet support 'LIMIT & IN/ALL/ANY/SOME subquery'
//
// The workaround is to call the SELECT with LIMIT and construct the IN clause manually
//
static const std::string  sel_alarm_opened_QUERY =
    " SELECT"
    "    v.id, v.rule_name, v.priority, v.state,"
    "    v.description, v.notification,"
    "    UNIX_TIMESTAMP(v.date_from), UNIX_TIMESTAMP(v.date_till),"
    "    v.id_asset_element "
    " FROM"
    "   v_bios_alert_all v"
    " WHERE v.date_till is NULL"
    " ORDER BY v.id";

static const std::string  sel_alarm_closed_QUERY =
    " SELECT"
    "    v.id, v.rule_name, v.priority, v.state,"
    "    v.description, v.notification,"
    "    UNIX_TIMESTAMP(v.date_from), UNIX_TIMESTAMP(v.date_till),"
    "    v.id_asset_element "
    " FROM"
    "   v_bios_alert_all v"
    " WHERE v.date_till is not NULL"
    " ORDER BY v.id";

static db_reply <std::vector<db_alert_t>>
    select_alert_all_template
        (tntdb::Connection  &conn,
         const std::string  &query)
{   
    LOG_START;
    std::vector<db_alert_t> item{};
    db_reply<std::vector<db_alert_t>> ret = db_reply_new(item);

    try {
        tntdb::Statement st = conn.prepareCached(query);
        tntdb::Result res = st.select();

        log_debug ("[v_bios_alert_all]: was %u rows selected", res.size());

        //FIXME: change to dbtypes.h
        uint64_t last_id = 0u;
        uint64_t curr_id = 0u;
        db_alert_t m{0, "", 0, 0, "", 0 , 0, 0, std::vector<m_dvc_id_t>{}};
        a_elmnt_id_t element_id = 42; // suppress the compiler may be unitialized warning
                                      // variable is never used unitialized, but gcc don't understand the r[8].get(element_id) does it
                                      // 42 is the Answer, so why not? ;-)

        for ( auto &r : res ) {

            r[0].get(curr_id);

            if (curr_id == last_id) {
                r[8].get(element_id);
                m.device_ids.push_back(element_id);
                continue;
            }

            if (!m.rule_name.empty()) {
                ret.item.push_back(m);
            }

            m = {0, "", 0, 0, "", 0 , 0, 0, std::vector<m_dvc_id_t>{}};

            r[0].get(m.id);
            r[1].get(m.rule_name);
            r[2].get(m.priority);
            r[3].get(m.alert_state);
            r[4].get(m.description);
            r[5].get(m.notification);
            r[6].get(m.date_from);
            r[7].get(m.date_till);

            bool isNotNull = r[8].get(element_id);
            if (isNotNull)
                m.device_ids.push_back(element_id);

            last_id = curr_id;
        }
        if (!m.rule_name.empty()) {
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


//=============================================================================
db_reply <std::vector<db_alert_t>>
    select_alert_all_opened
        (tntdb::Connection  &conn)
{
    return select_alert_all_template(conn, sel_alarm_opened_QUERY);
}


//=============================================================================
db_reply <std::vector<db_alert_t>>
    select_alert_all_closed
        (tntdb::Connection  &conn)
{
    return select_alert_all_template(conn, sel_alarm_closed_QUERY);
}


//=============================================================================
db_reply <std::vector<m_dvc_id_t>>
    select_alert_devices
        (tntdb::Connection &conn,
         m_alrt_id_t        alert_id)
{   
    LOG_START;
    std::vector<m_dvc_id_t> item{};
    db_reply<std::vector<m_dvc_id_t>> ret = db_reply_new(item);

    try {
        tntdb::Statement st = conn.prepareCached(
                " SELECT"
                "    v.device_id"
                " FROM"
                "   v_bios_alert_device v"
                " WHERE v.alert_id = :alert"
        );
        tntdb::Result res = st.set("alert", alert_id).
                               select();
        
        log_debug ("[t_bios_alert_device]: was %u rows selected", res.size());

        for ( auto &r : res ) {
            m_dvc_id_t device_id = 0;
            r[0].get(device_id);
            ret.item.push_back(device_id);
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

db_reply <db_alert_t>
    select_alert_last_byRuleName
        (tntdb::Connection &conn,
         const char *rule_name)
{   
    LOG_START;
    std::vector<m_dvc_id_t> dvc_ids{}; 
    db_alert_t m = {0, "", 0, 0, "", 0 , 0, 0, dvc_ids};

    db_reply<db_alert_t> ret = db_reply_new(m);

    try {
        tntdb::Statement st = conn.prepareCached(
                " SELECT"
                "   v.id, v.rule_name, v.priority, v.state,"
                "   v.descriprion, v.notification,"
                "   UNIX_TIMESTAMP(v.date_from), UNIX_TIMESTAMP(v.date_till)"
                " FROM"
                "   v_bios_alert v"
                " INNER JOIN"
                "       (SELECT"
                "          rule_name, max(dateFrom) AS date_max"
                "        FROM"
                "          v_bios_alert v"
                "        GROUP BY (rule_name)"
                "       ) v1"
                " WHERE v.rule_name = :rule AND"
                "   v.rule_name = v1.rule_name AND"
                "   v.date_from = v2.date_max"
        );
        tntdb::Row res = st.set("rule", rule_name).
                            selectRow();
        
        log_debug ("[t_bios_alert]: was %u rows selected", 1);

        res[0].get(m.id);
        res[1].get(m.rule_name);
        res[2].get(m.priority);
        res[3].get(m.alert_state);
        res[4].get(m.description);
        res[5].get(m.notification);
        res[6].get(m.date_from);
        res[7].get(m.date_till);
            
        auto reply_internal = select_alert_devices (conn, m.id);
        if ( reply_internal.status == 0 )
        {
            ret.status     = 0;
            ret.errtype    = DB_ERR;
            ret.errsubtype = DB_ERROR_BADINPUT; // TODO ERROR
            ret.msg        = "error in device selecting";
            log_error ("end: %s, %s", "ignore select", ret.msg);
            return ret;
        }
        ret.status = 1;
        LOG_END;
        return ret;
    } 
    catch(const tntdb::NotFound &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_NOTFOUND;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
    catch(const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

db_reply <db_alert_t>
    select_alert_byRuleNameDateFrom
        (tntdb::Connection &conn,
         const char *rule_name,
         int64_t     date_from)
{   
    LOG_START;
    std::vector<m_dvc_id_t> dvc_ids{}; 
    db_alert_t m = {0, "", 0, 0, "", 0 , 0, 0, dvc_ids};

    db_reply<db_alert_t> ret = db_reply_new(m);

    try {
        tntdb::Statement st = conn.prepareCached(
                " SELECT"
                "   v.id, v.rule_name, v.priority, v.state,"
                "   v.descriprion, v.notification,"
                "   UNIX_TIMESTAMP(v.date_from), UNIX_TIMESTAMP(v.date_till)"
                " FROM"
                "   v_bios_alert v"
                " WHERE v.rule_name = :rule AND"
                "   v.date_from = FROM_UNIXTIME(:date)"
        );
        tntdb::Row res = st.set("rule", rule_name).
                            set("date", date_from).
                            selectRow();
        
        log_debug ("[t_bios_alert]: was %u rows selected", 1);

        res[0].get(m.id);
        res[1].get(m.rule_name);
        res[2].get(m.priority);
        res[3].get(m.alert_state);
        res[4].get(m.description);
        res[5].get(m.notification);
        res[6].get(m.date_from);
        res[7].get(m.date_till);
            
        auto reply_internal = select_alert_devices (conn, m.id);
        if ( reply_internal.status == 0 )
        {
            ret.status     = 0;
            ret.errtype    = DB_ERR;
            ret.errsubtype = DB_ERROR_BADINPUT; // TODO ERROR
            ret.msg        = "error in device selecting";
            log_error ("end: %s, %s", "ignore select", ret.msg);
            return ret;
        }
        ret.status = 1;
        LOG_END;
        return ret;
    } 
    catch(const tntdb::NotFound &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_NOTFOUND;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
    catch(const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

db_reply_t
    update_alert_notification_byRuleName 
        (tntdb::Connection  &conn,
         m_alrt_ntfctn_t     notification,
         const char *rule_name)
{
    LOG_START;
    db_reply_t ret = db_reply_new();
    auto alert = select_alert_last_byRuleName
        (conn, rule_name);
    if ( alert.status == 0 )
    {
        ret.status     = alert.status;
        ret.errtype    = alert.errtype;
        ret.errsubtype = alert.errsubtype;
        ret.msg        = alert.msg;
        log_info ("end: can't find requested alert");
        return ret;
    }
    ret = update_alert_notification_byId(conn, notification, alert.item.id);
    LOG_END;
    return ret;
}

//=============================================================================
// FIXME: move those *_process functions to new file (other files too)

void process_alert(ymsg_t* out, char** out_subj,
                   ymsg_t* in, const char* in_subj)
{
    if( ! in || ! out ) return;

    LOG_START;
    
    if( in_subj ) { *out_subj = strdup(in_subj); }
    else { *out_subj = NULL; }
    
    log_debug("processing alert"); // FIXME: some macro
    ymsg_t *copy = ymsg_dup(in);
    assert(copy);
    
    // decode message
    char *rule = NULL, *devices = NULL, *desc = NULL;
    alert_priority_t priority;
    alert_state_t state;
    time_t since;
    if( bios_alert_decode( &copy, &rule, &priority, &state, &devices, &desc, &since) != 0 ) {
        ymsg_destroy(&copy);
        log_debug("can't decode message");
        LOG_END;
        return;
    }
    std::vector<std::string> devices_v;
    cxxtools::split(',', std::string(devices), std::back_inserter(devices_v));
    tntdb::Connection conn;
    try{        
        conn = tntdb::connect(url);
        db_reply_t ret;
        
        switch( (int)state ) {
        case ALERT_STATE_ONGOING_ALERT:
            // alert started
            ret = insert_new_alert(
                conn,
                rule,
                priority,
                state,
                ( desc ? desc : rule ),
                0,
                since,
                devices_v);
            ymsg_set_status( out, ret.status );
            break;
        case ALERT_STATE_NO_ALERT:
            //alarm end
            ret = update_alert_tilldate_by_rulename(
                conn,
                since,
                rule);
            ymsg_set_status( out, ret.status );
            break;
        }
        if(!ret.status) { log_error("Writting alert into the database failed"); }
    } catch(const std::exception &e) {
        LOG_END_ABNORMAL(e);
        ymsg_set_status( out, false );
    }
    if(rule) free(rule);
    if(devices) free(devices);
    LOG_END;
}

} // namespace persist

