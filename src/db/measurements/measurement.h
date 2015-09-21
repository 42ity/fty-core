/*
Copyright (C) 2015 Eaton

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

/*! \file   measurement.h
    \brief  high level db api for measurements
    \author Karol Hrdina <KarolHrdina@Eaton.com>
*/

#ifndef SRC_DB_MEASUREMENTS_MEASUREMENT_H__
#define SRC_DB_MEASUREMENTS_MEASUREMENT_H__

#include <string>
#include <functional>
#include <map>

#include <tntdb/connect.h>

#include "dbhelpers.h"
#include "db/types.h"
#include "dbtypes.h"

namespace persist
{

// -1 err/failure , 0 ok, 1 element_id not found, 2 topic not found    
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
        std::string& unit);

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
        int64_t& last_timestamp);

reply_t
select_measurements_sampled (
        tntdb::Connection &conn,
        uint64_t element_id,
        const char *topic,
        int64_t start_timestamp,
        int64_t end_timestamp,
        std::map <int64_t, double>& measurements,
        std::string& unit);

// this do not belong to this file
reply_t
select_device_name_from_element_id (
        tntdb::Connection &conn,
        uint64_t element_id,
        std::string& device_name);

reply_t
select_measurement_last_web_byElementId (
        tntdb::Connection &conn,
        const std::string& src,
        a_elmnt_id_t id,
        m_msrmnt_value_t& value,
        m_msrmnt_scale_t& scale,
        int minutes_back = 10);

reply_t
select_measurement_last_web_byTopicLike (
        tntdb::Connection &conn,
        const std::string& topic,
        m_msrmnt_value_t& value,
        m_msrmnt_scale_t& scale,
        int minutes_back = 10);

reply_t
select_measurement_last_web_byTopic (
        tntdb::Connection &conn,
        const std::string& topic,
        m_msrmnt_value_t& value,
        m_msrmnt_scale_t& scale,
        int minutes_back = 10,
        bool fuzzy = false);

int
    delete_measurements(
        tntdb::Connection &conn,
        m_msrmnt_tp_id_t   topic_id,
        m_msrmnt_id_t     &affected_rows
        );


int
    delete_measurement_topic(
        tntdb::Connection &conn,
        m_msrmnt_tp_id_t   topic_id,
        m_msrmnt_tp_id_t  &affected_rows
        );

db_reply<std::vector<db_msrmnt_t>>
    select_from_measurement_by_topic(
        tntdb::Connection &conn,
        const char        *topic);

db_reply_t
    delete_from_measurement_by_id(
        tntdb::Connection &conn,
        m_msrmnt_id_t      id);

int
    select_for_element_topics_all(
        tntdb::Connection &conn,
        a_elmnt_id_t       element_id,
        row_cb_f          &cb);


int
    insert_into_measurement_topic
        (tntdb::Connection &conn,
         m_dvc_id_t         monitor_element_id,
         const std::string &topic,
         const std::string &inits,
         m_msrmnt_tpc_id_t &rowid);


int
    insert_into_measurement_pure(
        tntdb::Connection &conn,
        m_msrmnt_value_t   value,
        m_msrmnt_scale_t   scale,
        m_msrmnt_tpc_id_t  topic_id,
        int64_t            time,
        m_msrmnt_id_t     &rowid);


int
    select_measurements_by_topic_id (
        tntdb::Connection &conn,
        m_msrmnt_tpc_id_t  topic_id,
        int64_t            start_timestamp,
        int64_t            end_timestamp,
        bool               left_interval_closed,
        bool               right_interval_closed,
        row_cb_f          &cb);


int
    select_current_measurement_by_element(
        tntdb::Connection &conn,
        a_elmnt_id_t       asset_id,
        row_cb_f          &cb);

} // namespace persist

#endif // SRC_DB_MEASUREMENTS_MEASUREMENT_H__

