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
    \brief  Functions to work with t_bios_measurement and t_bios_measurement_topic
    \author MichalVyskocil <MichalVyskocil@Eaton.com>
    \author AlenaChernikava <AlenaChernikava@Eaton.com>
 */

#ifndef SRC_PERSIST_MEASUREMENT_H_
#define SRC_PERSIST_MEASUREMENT_H_

#include <tntdb/connect.h>
#include <set>

#include "defs.h"
#include "dbhelpers.h"
#include "ymsg.h"

namespace persist {

class TopicCache {
    public:
        explicit TopicCache(size_t max = 1024):
            _cache{},
            _max{max}
        {};

        TopicCache (const TopicCache& other) = delete;
        TopicCache (const TopicCache&& other) = delete;
        TopicCache& operator=(TopicCache& other) = delete;
        TopicCache& operator=(TopicCache&& other) = delete;

        //\brief check if value is in cache or not
        bool has(const std::string& topic) const;

         //\brief add a key to cache
        void add(const std::string& topic);

    private:
        std::set<std::string> _cache;
        size_t _max;
};

/**
 * \brief Inserts measurements data into t_bios_measurement_topic, t_bios_measurement
 *
 * Inserts (topic, units) into t_bios_measurement_topic if given topic does not exists and
 * measurement data into t_bios_measurement
 *
 * \param conn        - the connection to database.
 * \param value       - the measured value
 * \param scale       - in which scale value exists
 * \param time        - the unix time where value has been measured
 * \param units       - the physical unit of value
 * \param cache       - (optional) the cache of already inserted topics
 *
 * \return db_reply_t with affected rows from t_bios_measurement insertion
 *                    or info about an error
 */
db_reply_t
    insert_into_measurement(
        tntdb::Connection &conn,
        const char        *topic,
        m_msrmnt_value_t   value,
        m_msrmnt_scale_t   scale,
        int64_t            time,
        const char        *units,
        const char        *device_name,
        TopicCache        &c);

// backward compatible function for a case where no cache is required
db_reply_t
    insert_into_measurement(
        tntdb::Connection &conn,
        const char        *topic,
        m_msrmnt_value_t   value,
        m_msrmnt_scale_t   scale,
        int64_t            time,
        const char        *units,
        const char        *device_name);

db_reply<std::vector<db_msrmnt_t>>
    select_from_measurement_by_topic(
        tntdb::Connection &conn,
        const char        *topic);

db_reply_t
    delete_from_measurement_by_id(
        tntdb::Connection &conn,
        m_msrmnt_id_t      id);


} //namespace persist

#endif // SRC_PERSIST_MEASUREMENT_H_
