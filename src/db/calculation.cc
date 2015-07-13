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

#include <tntdb/result.h>
#include <algorithm>
#include "log.h"
#include "db/alerts/alert_basic.h"
#include "dbpath.h"
#include "measurement.h"
#include "db/measurements.h"
#include "asset_types.h"
#include "assetcrud.h"
#include "dbhelpers.h"

namespace persist {


reply_t
    select_alertDates_byRuleName_byInterval_byDcId
        (tntdb::Connection &conn,
         const std::string &rule_name,
         int64_t start_date,
         int64_t end_date,
         a_elmnt_id_t dc_id,
         std::vector <int64_t> &start,
         std::vector <int64_t> &end)
{
    LOG_START;
    reply_t ret;
    // input parameters control
    if ( start_date >= end_date )
    {
        log_debug ("start_date >= end_date");
        ret.rv = -1;
        return ret;
    }
    log_debug ("input parameters are correct");

    log_debug ("  rule_name = '%s'", rule_name.c_str());
    log_debug ("  start_date = %" PRIi64, start_date);
    log_debug ("  end_date = %" PRIi64, end_date);
    log_debug ("  dc_id = %" PRIu32, dc_id);

    try{
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "   GREATEST(UNIX_TIMESTAMP(date_from), :A) a,"
            "   LEAST(UNIX_TIMESTAMP(date_till), :B) b"
            " FROM t_bios_alert"
            " WHERE"
            "   NOT ( ( UNIX_TIMESTAMP(date_from) < :A AND"
            "           UNIX_TIMESTAMP(date_till) <= :A"
            "         ) OR"
            "         ( UNIX_TIMESTAMP(date_from) > :B AND"
            "           UNIX_TIMESTAMP(date_till) >= :B"
            "         )"
            "       ) AND"
            "   rule_name LIKE :rn AND"
            "   dc_id = :id"
            " ORDER BY a"
        );

        tntdb::Result res = st.set("A", start_date).
                               set("B", end_date).
                               set("rn", rule_name).
                               set("id", dc_id).
                               select();
        log_debug ("[t_bios_alert]: was selected %"
                                    PRIu32 " rows", res.size());
        for ( auto &row : res )
        {
            int64_t a = 0;
            row[0].get(a);
            start.push_back(a);
            row[1].get(a);
            end.push_back(a);
        }
        ret.rv = 0;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.rv = 2;
        start.clear();
        end.clear();
        LOG_END_ABNORMAL(e);
        return ret;
    }
    catch (...) {
        ret.rv = -2;
        start.clear();
        end.clear();
        log_error("Unknown exception caught!");
        return ret;
    }
}


int
    calculate_outage_byInerval_byDcId
        (tntdb::Connection &conn,
         int64_t start_date,
         int64_t end_date,
         a_elmnt_id_t dc_id,
         int64_t &outage) // outage in seconds
{
    LOG_START;
    std::vector <int64_t> s{};
    std::vector <int64_t> e{};
    reply_t ret = select_alertDates_byRuleName_byInterval_byDcId
                (conn, "upsonbattery%", start_date, end_date, dc_id, s, e);
    if ( ret.rv != 0 )
        return ret.rv;

    // true = start date
    // false = end date
    std::vector <std::pair<int64_t, bool>> timeline{};
    for ( auto &one : s )
        timeline.push_back(std::make_pair(one, true));
    for ( auto &one : e )
        timeline.push_back(std::make_pair(one, false));
    std::sort (timeline.begin(), timeline.end(), 
            [&] (std::pair<int64_t, bool>i,std::pair<int64_t, bool> j) 
                -> bool { return i.first == j.first ? 
                                        (i.second == true) : 
                                        (i.first < j.first); }
         );

    std::vector <std::pair<int64_t, bool>> st{};
    outage = 0;
    // process timeline and calc an outage
    // solve a paranthesis problem
    // 'start date' = (
    // 'end date' = )
    // timeline = ((()()())) () (()())
    // outage   = ((()()()))+()+(()())
    std::string parenthesis = "";
    for ( auto &element : timeline )
    {
        // if it is start date, then add to the stack
        // if it is end date, then remove the last start date from the stack
        if ( element.second == true )
        {
            parenthesis = parenthesis +"(";
            st.push_back(element);
        }
        else
        {
            parenthesis = parenthesis +")";
            if ( st.size() == 0 )
                // this should never happen, but if we do, then end correctly
                return -6;  
            // get last element
            auto last = st.back();
            //delete it from stack
            st.pop_back();
            // if it was the "biggest" paranthesis then calc time
            if  ( st.empty() )
            {
                parenthesis = parenthesis + "+";
                outage = outage + (element.first - last.first);
            }
        }
    }
    log_debug (" chain: %s", parenthesis.c_str());
    if ( st.size() != 0 )
        // this should never happen, but if we do, then end correctly
        return -5;
    else
        return 0;
}


reply_t
    insert_outage
        (tntdb::Connection &conn,
         const char        *dc_name,
         m_msrmnt_value_t   value,
         int64_t            timestamp)
{
    std::string topic = "outage@" + std::string(dc_name, strlen(dc_name));
    // TODO get rid of old type
    db_reply_t ret = insert_into_measurement
            (conn, topic.c_str(), value, 0, timestamp, "s", dc_name);

    // TODO: get rid of this after refactoring
    reply_t r;
    r.affected_rows = ret.affected_rows;
    r.row_id = ret.rowid;

    if ( ret.status == 1 )
        r.rv = 0;
    else
        r.rv = ret.errsubtype;
    return r;
}


static
void
    get_interval
        (int64_t &start_date,
         int64_t &end_date)
{
    // get current date
    time_t sec = time (NULL);
    struct tm *new_time = gmtime ( &sec );

    // date 00:00:00
    new_time->tm_hour = 0;
    new_time->tm_min  = 0;
    new_time->tm_sec  = 0;
    end_date = mktime (new_time);

    // date-1day 00:00:00
    new_time->tm_mday--;
    start_date = mktime (new_time);
}


int
    compute_new_dc_outage(void)
{
    LOG_START;
    // ASSUMPTION
    // all previous days are already calculated
    // TODO get rid of this assumption

    // calculate last interval to calculate
    int64_t start_date = 0;
    int64_t end_date = 0;
    get_interval (start_date, end_date);

    try{
        // open a connection to db
        tntdb::Connection conn = tntdb::connectCached(url);

        // select all datacenters
        db_reply < std::vector <db_a_elmnt_t> > dcs =
            select_asset_elements_by_type (conn, asset_type::DATACENTER);

        // do work for every dc
        for ( auto &dc :dcs.item )
        {
            // outage in seconds
            int64_t outage = 0;
            // calculate new value
            int rv = calculate_outage_byInerval_byDcId
                (conn, start_date, end_date, dc.id, outage);
            if ( rv == 0 )
            {
                // Attention: outage for the
                // period 18.01.2015 00:00:00 - 19.01.2015 00:00:00
                // is a measuement at the ned date 19.01.2015 00:00:00
                // insert new value
                reply_t ret = insert_outage
                        (conn, dc.name.c_str(), outage, end_date);
                if ( ret.rv != 0 )
                    log_debug ("FAIL: outage %" PRIi64 " for dc " \
                            "( %" PRIi32 ") was not inserted for "\
                            "the period from %" PRIi64 " to %" PRIi64,
                            outage, dc.id, start_date, end_date);
                else
                    log_debug ("SUCCESS: outage %" PRIi64 " for dc " \
                            "( %" PRIi32 ")  was inserted for " \
                            "the period from %" PRIi64 " to %" PRIi64,
                            outage, dc.id, start_date, end_date);
            }
            else
                log_error ("FAIL: outage for dc ( %" PRIi32 ") " \
                        "was not calculated for the period " \
                        "from %" PRIi64 " to %" PRIi64, dc.id,
                        start_date, end_date);
        }
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return 2;
    }
    catch (...) {
        LOG_END;
        log_error("Unknown exception caught!");
        return -2;
    }
    LOG_END;
    return 0;
}


reply_t 
    select_outage_byDC_byInterval
        (tntdb::Connection &conn,
         a_elmnt_id_t dc_id,
         int64_t start_date,
         int64_t end_date,
         std::map <int64_t, double> &measurements)
{
    LOG_START;
    log_debug ("  dc_id = %" PRIi32, dc_id);

    // time, value
    std::string unit{};

    reply_t ret = select_measurements (
        conn,
        dc_id,
        "outage",
        start_date,
        end_date,
        true,
        true,
        measurements,
        unit);
    log_debug (" selected %u", measurements.size());
    LOG_END;
    return ret;
}


int
    calculate_total_outage_byDcId
        (tntdb::Connection &conn,
         a_elmnt_id_t dc_id,
         int64_t &sum)
{
    LOG_START;
    sum = 0;
    // ASSUMPTION: start_date should be stored in ext_attrubutes for the DC
    // with the key "dc_start_date"
    // TODO: implement it
    // current implementation: hardcoded value
    time_t start_date = 1435708800; //01.07.2015

    // select start_date

    // get end_date(NOW())
    time_t end_date = time (NULL);

    // get values, that we are going to sum up
    std::map <int64_t, double> measurements{};
    reply_t ret = select_outage_byDC_byInterval 
        (conn, dc_id, start_date, end_date, measurements);
    // rv = 0 everything is fine
    // rv = 2 no topic -> no measurements were found -> no outage
    if  ( ( ret.rv != 0 ) && ( ret.rv != 2 ) )
    {
        log_debug (" problems with selecting outage_by_interval" \
                     " rv = %" PRIi32, ret.rv);
        return ret.rv;
    }

    // if values were selected, then sum up them
    sum = 0;
    for ( auto &one_outage : measurements )
    {
        sum += one_outage.second;
    }
    log_debug (" outage = %" PRIi64, sum);
    LOG_END;
    return 0;
}

static
int
    calculate_total_running_time 
        (tntdb::Connection &conn,
         a_elmnt_id_t dc_id,
         int64_t &sum)
{
    sum = time (NULL) -  1435708800; // NOW  - 01.07.2015 in seconds
    log_debug (" total_time = %" PRIi64, sum);
    return 0;
}

int
    calculate_uptime_total_byDcId
        (tntdb::Connection &conn,
         a_elmnt_id_t dc_id,
         int64_t &uptime,
         int64_t &total)
{
    int64_t outage = 0;
    int rv = calculate_total_outage_byDcId
        (conn, dc_id, outage);
    if ( rv )
        return rv;
    rv = calculate_total_running_time 
        (conn, dc_id, total);
    uptime = total - outage;
    return rv;
}



} // end namespace
