/*
Copyright (C) 2015 Eaton
 
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

/*!
 \file measurement.h
 \brief high level db api for measurements
 \author Karol Hrdina <KarolHrdina@eaton.com>
*/

#ifndef SRC_DB_MEASUREMENTS_MEASUREMENT_H__
#define SRC_DB_MEASUREMENTS_MEASUREMENT_H__

#include <tntdb/connect.h>
#include <map>
#include <string>

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
        m_msrmnt_scale_t& scale);

reply_t
select_measurement_last_web_byTopicLike (
        tntdb::Connection &conn,
        const std::string& topic,
        m_msrmnt_value_t& value,
        m_msrmnt_scale_t& scale);

} // namespace persist

#endif // SRC_DB_MEASUREMENTS_MEASUREMENT_H__

