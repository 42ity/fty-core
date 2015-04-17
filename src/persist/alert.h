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

/*! \file alert.h
    \brief Pure DB API for CRUD operations on slerts

    \author Alena Chernikava <alenachernikava@eaton.com>
*/

#ifndef SRC_PERSIST_ALERT
#define SRC_PERSIST_ALERT

#include "dbtypes.h"
#include "dbhelpers.h"

db_reply_t
    insert_into_alert 
        (tntdb::Connection  &conn,
         const char         *rule_name,
         a_elmnt_pr_t        priority,
         m_alrt_state_t      alert_state,
         const char         *description,
         m_alrt_ntfctn_t     notification,
         int64_t             date_from);


db_reply_t
    update_alert_notification 
        (tntdb::Connection  &conn,
         m_alrt_ntfctn_t     notification,
         m_alrt_id_t         id);


db_reply_t
    update_alert_tilldate 
        (tntdb::Connection  &conn,
         int64_t             date_till,
         m_alrt_id_t         id);


db_reply_t
    delete_alert
        (tntdb::Connection &conn,
         m_alrt_id_t id);


db_reply_t
    insert_into_alert_devices 
        (tntdb::Connection  &conn,
         m_alrt_id_t               alert_id,
         std::vector <std::string> device_names);


db_reply_t
    insert_into_alert_device
        (tntdb::Connection &conn,
         m_alrt_id_t        alert_id,
         const char        *device_name);


db_reply_t
    delete_alert
        (tntdb::Connection &conn,
         m_alrtdvc_id_t    id);

#endif //SRC_PERSIST_ALERT
