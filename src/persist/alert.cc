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
    \brief Pure DB API for CRUD operations on slerts

    \author Alena Chernikava <alenachernikava@eaton.com>
*/

#include <tntdb/connect.h>
#include <tntdb/row.h>
#include <tntdb/result.h>
#include <tntdb/error.h>
#include <tntdb/transaction.h>

#include "log.h"
#include "defs.h"
#include "alert.h"

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
            "   (rule_name, priority, state, descriprion, notification, from, till)"
            " SELECT"
            "    :rule, :priority, :state, :desc, :note, :from, NULL"
            " FROM"
            "   t_empty"
            " WHERE NOT EXISTS"
            "   ("
            "       SELECT"
            "           id"
            "       FROM"
            "           t_bios_alert"
            "       WHERE"
            "           till is NULL AND"
            "           rule_name = :rule"
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
    update_alert_notification 
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
            " SET till = :till"
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
    delete_alert
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

    for (auto &device_name : device_names )
    {
        auto reply_internal = insert_into_alert_device
                                (conn, alert_id, device_name.c_str());
        if ( reply_internal.status == 1 )
            ret.affected_rows++;
    }

    if ( ret.affected_rows == device_names.size() )
    {
        ret.status = 1;
        log_debug ("all ext attributes were inserted successfully");
        LOG_END;
        return ret;
    }
    else
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "not all attributes were inserted";
        log_error ("end: %s", ret.msg);
        return ret;
    }
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
            "   v1.name = :name"
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
    delete_alert_device
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
        ret.rowid = conn.lastInsertId();
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
