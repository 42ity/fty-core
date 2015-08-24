/*
Copyright (C) 2014-2015 Eaton

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <tntdb/row.h>
#include <tntdb/result.h>
#include <tntdb/error.h>
#include <tntdb/transaction.h>
#include "log.h"
#include "db/alerts/alert_basic.h"
#include "assetcrud.h"
#include "db/assets.h"

namespace persist {

//=============================================================================
db_reply_t
    insert_into_alert
        (tntdb::Connection  &conn,
         const char         *rule_name,
         a_elmnt_pr_t        priority,
         m_alrt_state_t      alert_state,
         const char         *description,
         m_alrt_ntfctn_t     notification,
         int64_t             date_from,
         a_elmnt_id_t        dc_id)
{
    LOG_START;
    log_debug ("  rule_name = '%s'", rule_name);
    log_debug ("  priority = %" PRIu16, priority);
    log_debug ("  alert_state = %" PRIu16, alert_state);
    log_debug ("  description = '%s'", description);
    log_debug ("  date_from = %" PRIi64, date_from);
    log_debug ("  notification = %" PRIu16, notification);
    log_debug ("  dc_id = %" PRIu32, dc_id);

    db_reply_t ret = db_reply_new();

    // input parameters control
    if ( !is_ok_rule_name (rule_name) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "rule name is invalid";
        log_error ("end: ignore insert '%s'", ret.msg.c_str());
        return ret;
    }
    if ( !is_ok_priority (priority) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "unsupported value of priority";
        log_error ("end: ignore insert '%s'", ret.msg.c_str());
        return ret;
    }
    if ( !is_ok_alert_state (alert_state) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "alert state is invalid";
        log_error ("end: ignore insert '%s'", ret.msg.c_str());
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
            "   notification, date_from, dc_id)"
            " SELECT"
            "    :rule, :priority, :state, :desc, :note, :from, :dc_id"
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
                               set("dc_id", dc_id).
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
         m_alrt_id_t         alert_id)
{
    LOG_START;
    log_debug ("  notification = %" PRIu16, notification);
    log_debug ("  alert_id = %" PRIu32, alert_id);

    db_reply_t ret = db_reply_new();
    
    try{
        tntdb::Statement st = conn.prepareCached(
            " UPDATE"
            "   t_bios_alert"
            " SET notification = (notification | :note)"  // a bitwire OR
            " WHERE  id = :id"
        );
   
        ret.affected_rows = st.set("id", alert_id).
                               set("note", notification).
                               execute();
        log_debug ("[t_bios_alert]: was updated %" 
                                    PRIu64 " rows", ret.affected_rows);
        ret.status = 1;
        // update statement doesn't trigger a lastinsertedid functionality
        // use from the paramenter
        if ( ret.affected_rows != 0 )
        {
            ret.rowid = alert_id;
        }
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
            " SET date_till = :till,"
            "     id=LAST_INSERT_ID(id)"
            " WHERE rule_name = :rule AND"
            "   date_till is NULL"
        );
   
        ret.affected_rows = st.set("rule", rule_name).
                               set("till", date_till).
                               execute();
        log_debug ("[t_bios_alert]: was updated %" 
                                    PRIu64 " rows", ret.affected_rows);
        ret.rowid = conn.lastInsertId();
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
         m_alrt_id_t         alert_id)
{
    LOG_START;
    log_debug ("  tilldate = %" PRIi64, date_till);
    log_debug ("  id = %" PRIu32, alert_id);

    db_reply_t ret = db_reply_new();
    
    try{
        tntdb::Statement st = conn.prepareCached(
            " UPDATE"
            "   t_bios_alert"
            " SET date_till = :till"
            " WHERE  id = :id"
        );
   
        ret.affected_rows = st.set("id", alert_id).
                               set("till", date_till).
                               execute();
        log_debug ("[t_bios_alert]: was updated %" 
                                    PRIu64 " rows", ret.affected_rows);
        if ( ret.affected_rows != 0 )
        {
            ret.rowid = alert_id;
        }
        
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
         m_alrt_id_t alert_id)
{
    LOG_START;
    log_debug ("  id = %" PRIu32, alert_id);

    db_reply_t ret = db_reply_new();

    try{
        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            "   t_bios_alert"
            " WHERE"
            "   id = :id"
        );
   
        ret.affected_rows = st.set("id", alert_id).
                               execute();
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
        (tntdb::Connection        &conn,
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
        log_error ("end: ignore insert '%s'", ret.msg.c_str());
        return ret;
    }
    if ( device_names.size() == 0 )
    {
        // actually, if there is nothing to insert, then insert was ok
        ret.status = 1;
        log_info ("end: ignore insert 'noting to insert'");
        return ret;
    }
    log_debug ("input parameters are correct");

    ret.status = 1;
    // try to insert all devices
    // If some erorr occures, then save info about it and continue
    for (auto &device_name : device_names )
    {
        auto reply_internal = insert_into_alert_device
                                (conn, alert_id, device_name.c_str());
        if ( ( reply_internal.status == 1 ) && 
             ( reply_internal.affected_rows == 1 ) )
            ret.affected_rows++;
        ret.status = ret.status && reply_internal.status;
        if ( ret.errtype == 0 )
            ret.errtype = reply_internal.errtype;
        if ( ret.errsubtype == 0 )
            ret.errsubtype = reply_internal.errsubtype;
        if ( ret.msg.empty() )
            ret.msg = reply_internal.msg;
    }
    LOG_END;
    return ret;
}

//=============================================================================
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
        log_error ("end: ignore insert '%s'", ret.msg.c_str());
        return ret;
    }
    if ( !is_ok_name(device_name) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "value of device_name is invalid";
        log_error ("end: ignore insert '%s'", ret.msg.c_str());
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
         m_alrt_id_t         alert_id)
{
    LOG_START;
    log_debug ("  id = %" PRIu32, alert_id);

    db_reply_t ret = db_reply_new();

    try{
        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            "   t_bios_alert_device"
            " WHERE"
            "   alert_id = :id"
        );
   
        ret.affected_rows = st.set("id", alert_id).
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
int
    delete_device_from_alert_device_all
        (tntdb::Connection &conn,
         m_dvc_id_t         device_id,
         m_alrtdvc_id_t   &affected_rows)
{
    LOG_START;
    log_debug ("  device_id = %" PRIu32, device_id);

    try{
        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            "   t_bios_alert_device"
            " WHERE"
            "   device_id = :id"
        );

        affected_rows = st.set("id", device_id).
                           execute();
        log_debug ("[t_bios_alert_device]: was deleted %"
                                        PRIu32 " rows", affected_rows);
        LOG_END;
        return 0;
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return -1;
    }
}

//=============================================================================
// TODO this function has a logic and represents one operation.
// Consider removing conn parametr and moving the function somewhere
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

    // Assumption: devices are in the same DC, so
    // we can take a random one

    // find a DC alert belongs to
    a_elmnt_id_t dc_id = 0;
    if ( !device_names.empty() )
    {
        db_reply <db_a_elmnt_t> rep = select_asset_element_by_name
            (conn, device_names.at(0).c_str());
        if ( rep.status != 1 )
        {
            trans.rollback();
            log_info ("end: alert was not inserted (fail in selecting devices");
            db_reply_t ret;
            ret.msg = rep.msg;
            ret.errtype = rep.errtype;
            ret.errsubtype = rep.errsubtype;
            ret.status = rep.status;
            return ret;
        }
        // some element was found
        reply_t r = select_dc_of_asset_element (conn, rep.item.id, dc_id);
        if ( r.rv != 0 )
        {
            db_reply_t ret = db_reply_new();
            ret.status = 0;
            ret.errsubtype = r.rv;
            log_debug ("problems with selecting DC");
            return ret;
        }
    }

    auto reply_internal1 = insert_into_alert
        (conn, rule_name, priority, alert_state, description,
         notification, date_from, dc_id);
    if ( ( reply_internal1.status == 0 ) ||
         ( reply_internal1.affected_rows == 0 ) )
    {
        trans.rollback();
        log_info ("end: alert was not inserted (fail in alert");
        return reply_internal1;
    }
    auto alert_id = reply_internal1.rowid;

    auto reply_internal2 = insert_into_alert_devices
                                    (conn, alert_id, device_names);
    if ( ( reply_internal2.status == 0 ) ||
         ( reply_internal2.affected_rows != device_names.size() ) )
    {
        trans.rollback();
        log_info ("end: alert was not inserted (fail in alert devices");
        return reply_internal2;
    }
    trans.commit();
    LOG_END;
    return reply_internal1;
}

//=============================================================================
static const std::string  sel_alert_opened_QUERY =
    " SELECT"
    "    v.id, v.rule_name, v.priority, v.state,"
    "    v.description, v.notification,"
    "    v.date_from, v.date_till,"
    "    v.id_asset_element, v.type_name, v.subtype_name"
    " FROM"
    "   v_web_alert_all v"
    " WHERE v.date_till is NULL"
    " ORDER BY v.id";

static const std::string  sel_alert_closed_QUERY =
    " SELECT"
    "    v.id, v.rule_name, v.priority, v.state,"
    "    v.description, v.notification,"
    "    v.date_from, v.date_till,"
    "    v.id_asset_element, v.type_name, v.subtype_name"
    " FROM"
    "   v_web_alert_all v"
    " WHERE v.date_till is not NULL"
    " ORDER BY v.id";

/*
 * \brief Selects all fields from v_web_alert_all by different 
 *          conditions specified in query.
 *
 * TODO: get rid of ORDER BY in queries. 
 * Redo a selection of devices that where used in alert evaluation
 */
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

        log_debug ("[v_web_alert_all]: was %u rows selected", res.size());

        //FIXME: change to dbtypes.h
        uint64_t last_id = 0u;
        uint64_t curr_id = 0u;
        db_alert_t m{0, "", 0, 0, "", 0 , 0, 0, "", "",
                std::vector<m_dvc_id_t>{}};
        a_elmnt_id_t element_id = 42; 
        // suppress the compiler may be unitialized warning
        // variable is never used unitialized, 
        // but gcc don't understand the r[8].get(element_id) does it
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

            m = {0, "", 0, 0, "", 0 , 0, 0, "","",std::vector<m_dvc_id_t>{}};

            r[0].get(m.id);
            r[1].get(m.rule_name);
            r[2].get(m.priority);
            r[3].get(m.alert_state);
            r[4].get(m.description);
            r[5].get(m.notification);
            r[6].get(m.date_from);
            r[7].get(m.date_till);
            r[9].get(m.type_name);
            r[10].get(m.subtype_name);

            log_debug ("rule_name is %s", m.rule_name.c_str());
            bool isNotNull = r[8].get(element_id);
            if (isNotNull)
                m.device_ids.push_back(element_id);

            last_id = curr_id;
        }
        if (!m.rule_name.empty()) {
            ret.item.push_back(m);
        }
        ret.status = 1;
        LOG_END;
        return ret;
    } catch(const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        ret.item.clear();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
db_reply <std::vector<db_alert_t>>
    select_alert_all_opened
        (tntdb::Connection  &conn)
{
    return select_alert_all_template(conn, sel_alert_opened_QUERY);
}

//=============================================================================
db_reply <std::vector<db_alert_t>>
    select_alert_all_closed
        (tntdb::Connection  &conn)
{
    return select_alert_all_template(conn, sel_alert_closed_QUERY);
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
        LOG_END;
        return ret;
    } catch(const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        ret.item.clear();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
db_reply <db_alert_t>
    select_alert_last_byRuleName
        (tntdb::Connection &conn,
         const char *rule_name)
{   
    LOG_START;
    std::vector<m_dvc_id_t> dvc_ids{}; 
    db_alert_t m = {0, "", 0, 0, "", 0 , 0, 0,"","", dvc_ids};

    db_reply<db_alert_t> ret = db_reply_new(m);

    try {
        tntdb::Statement st = conn.prepareCached(
                " SELECT"
                "   v.id, v.rule_name, v.priority, v.state,"
                "   v.description, v.notification,"
                "   v.date_from, v.date_till"
                " FROM"
                "   v_bios_alert v"
                " INNER JOIN"
                "       (SELECT"
                "          rule_name, max(date_from) AS date_max"
                "        FROM"
                "          v_bios_alert v"
                "        GROUP BY (rule_name)"
                "       ) v1"
                " WHERE v.rule_name = :rule AND"
                "   v.rule_name = v1.rule_name AND"
                "   v.date_from = v1.date_max"
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
            log_error ("end: %s, %s", "ignore select", ret.msg.c_str());
            return ret;
        }
        m.device_ids = reply_internal.item;
        ret.item = m;
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

//=============================================================================
db_reply <db_alert_t>
    select_alert_byRuleNameDateFrom
        (tntdb::Connection &conn,
         const char *rule_name,
         int64_t     date_from)
{
    LOG_START;
    std::vector<m_dvc_id_t> dvc_ids{};
    db_alert_t m = {0, "", 0, 0, "", 0 , 0, 0,"","", dvc_ids};

    db_reply<db_alert_t> ret = db_reply_new(m);

    try {
        tntdb::Statement st = conn.prepareCached(
                " SELECT"
                "   v.id, v.rule_name, v.priority, v.state,"
                "   v.description, v.notification,"
                "   v.date_from, v.date_till"
                " FROM"
                "   v_bios_alert v"
                " WHERE v.rule_name = :rule AND"
                "   v.date_from = :date"
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
            log_error ("end: %s, %s", "ignore select", ret.msg.c_str());
            return ret;
        }
        m.device_ids = reply_internal.item;
        ret.item = m;
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

//=============================================================================
db_reply_t
    update_alert_notification_byRuleName 
        (tntdb::Connection  &conn,
         m_alrt_ntfctn_t     notification,
         const char         *rule_name)
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

int
    select_alert_by_element_all(
        tntdb::Connection &conn,
        a_elmnt_id_t       element_id,
        std::function<void(
                const tntdb::Row&
                )>& cb)
{
    LOG_START;
    try {
        tntdb::Statement st = conn.prepareCached(
                " SELECT "
                "   va.id, va.rule_name, va.priority, va.state, "
                "   va.description, va.notification, "
                "   va.date_from, va.date_till "
                " FROM "
                "   v_bios_alert va "
                " INNER JOIN "
                "   v_bios_alert_device vad "
                " ON "
                "   ( va.id = vad.alert_id ) "
                " INNER JOIN "
                "   v_bios_monitor_asset_relation vmar "
                " ON "
                "   ( vad.device_id = vmar.id_discovered_device AND "
                "     vmar.id_asset_element = :element ) "
        );
        tntdb::Result res = st.set("element", element_id).
                               select();

        for (const auto& r: res) {
            cb(r);
        }
        LOG_END;
        return 0;
    }
    catch(const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return -1;
    }
}

} // namespace persist
