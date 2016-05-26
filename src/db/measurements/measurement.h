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
    \author Alena Chernikava <AlenaChernikava@Eaton.com>
    \brief  functionality to manipulate with measurements
*/

#ifndef SRC_DB_MEASUREMENTS_MEASUREMENT_H__
#define SRC_DB_MEASUREMENTS_MEASUREMENT_H__

#include <string>
#include <functional>
#include <set>

#include <tntdb/connect.h>

#include "dbhelpers.h"
#include "db/types.h"
#include "dbtypes.h"

namespace persist
{
// this do not belong to this file
reply_t
select_device_name_from_element_id (
        tntdb::Connection &conn,
        uint64_t element_id,
        std::string& device_name);
int
    delete_measurements(
        tntdb::Connection& conn,
        std::set<m_msrmnt_tpc_id_t> topics);


int
    delete_measurement_topics(
        tntdb::Connection& conn,
        std::set<m_msrmnt_tpc_id_t> topics);

int
    select_for_element_topics_all(
        tntdb::Connection &conn,
        a_elmnt_id_t       element_id,
        row_cb_f          &cb);

} // namespace persist

#endif // SRC_DB_MEASUREMENTS_MEASUREMENT_H__

