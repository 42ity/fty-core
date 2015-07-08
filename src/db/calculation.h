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

/*! \file 
    \brief 

    \author Alena Chernikava <alenachernikava@eaton.com>
*/
#include <tntdb/connect.h> // conn
#include <string> //string
#include "defs.h"
#include "utils.h"
#include "types.h" // reply_t
#include "measurement.h"
#include "db/measurements.h"

namespace persist {


reply_t
    select_alertDates_byRuleName_byInterval_byDcId
        (tntdb::Connection &conn,
         const std::string &rule_name,
         int64_t start_date,
         int64_t end_date,
         a_elmnt_id_t dc_id,
         std::vector <int64_t> &start,
         std::vector <int64_t> &end);

int
    calculate_outage_byInerval_byDcId
        (tntdb::Connection &conn,
         int64_t start_date,
         int64_t end_date,
         a_elmnt_id_t dc_id,
         int64_t &outage); // outage in seconds

reply_t
    insert_outage
        (tntdb::Connection &conn,
         const char        *dc_name,
         m_msrmnt_value_t   value,
         int64_t            timestamp);


int
    compute_new_dc_outage(void);


reply_t 
    select_outage_byDC_byInterval
        (tntdb::Connection &conn,
         a_elmnt_id_t dc_id,
         int64_t start_date,
         int64_t end_date,
         std::map <int64_t, double> &measurements);
    

int
    calculate_total_outage_byDcId
        (tntdb::Connection &conn,
         a_elmnt_id_t dc_id,
         double &sum);

} // end namespace
