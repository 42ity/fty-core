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

#include <string>
#include <stdexcept>
#include <map>
#include <tntdb/connect.h>

#include "cm-web.h"

#include "agents.h"
#include "defs.h"
#include "log.h"
#include "str_defs.h"
#include "utils.h"
#include "utils_ymsg.h"
#include "utils_ymsg++.h"
#include "db/measurements.h"
#include "cm-utils.h"
#include "cleanup.h"
#include "utils++.h"
#include "dbpath.h"

namespace computation {
namespace web {

void
process
(bios_agent_t *agent, ymsg_t *message_in, const char *sender, ymsg_t **message_out) {
    assert (agent);
    assert (message_in);
    assert (sender);
    assert (message_out);

    int64_t start_ts = -1, end_ts = -1;
    char *type = NULL, *step = NULL, *source = NULL;
    uint64_t element_id = 0;

    try {
        tntdb::Connection conn = tntdb::connectCached (url);
        if ( !conn.ping ()) {
            throw std::runtime_error ("tntdb::connectCached () failed.");
        }

        *message_out = ymsg_new (YMSG_REPLY);
        assert (*message_out);

        // Decode web average request
        int rv = bios_web_average_request_extract (message_in, &start_ts, &end_ts, &type, &step, &element_id, &source);
        if (rv != 0) {
            log_error ("bios_web_average_request_extract () failed.");
            ymsg_set_status (*message_out, false);
            ymsg_set_errmsg (*message_out, "Internal error: Protocol failure.");
            return;       
        }
        if (!is_average_type_supported (type) || !is_average_step_supported (step)) {
            log_error ("Requested average 'type' or 'step' not supported.");
            ymsg_set_status (*message_out, false);
            ymsg_set_errmsg (*message_out, "Requested average 'type' or 'step' not supported.");
            return;
        }
        
        // First, try to request the averages
        std::string unit;
        int64_t last_average_ts;
        std::map<int64_t, double> averages;
        if (!request_averages (conn, element_id, source, type, step, start_ts, end_ts, averages, unit, last_average_ts, *message_out)) {
            return;
        } 
        std::string json_out (BIOS_WEB_AVERAGE_REPLY_JSON_TMPL); // resulting output json           
        json_out.replace (json_out.find ("##SOURCE##"), strlen ("##SOURCE##"), source);
        json_out.replace (json_out.find ("##STEP##"), strlen ("##STEP##"), step);
        json_out.replace (json_out.find ("##TYPE##"), strlen ("##TYPE##"), type);
        json_out.replace (json_out.find ("##ELEMENT_ID##"), strlen ("##ELEMENT_ID##"), std::to_string (element_id));
        json_out.replace (json_out.find ("##START_TS##"), strlen ("##START_TS##"), std::to_string (start_ts));
        json_out.replace (json_out.find ("##END_TS##"), strlen ("##END_TS##"), std::to_string (end_ts));

        std::map <int64_t, double> samples;

        int64_t start_sampled_ts = 0;
        std::string data_str; // json; member 'data'
        int comma_counter = 0;

        // Check if returned averages are complete
        if (averages.empty ()) {
            log_info ("Requested averages: Empty");
            if (last_average_ts < start_ts) {
                // requesting stored averages returned an empty set && last stored average's timestamp < start of requested interval
                // => we need to extend compute everything from the last stored average's timestamp 

                // NOTE (current persistence limitation):
                // Timestamp is stored as DATETIME, function FROM_UNIXTIMESTAMP () can't handle negative values,
                // the lowest possible unixtime representable as datetime is '0'.
                if (last_average_ts == INT32_MIN) {
                    log_info ("There is no average stored yet (for given topic). Assigning timestamp of start of sampled data request value of '0'.");
                    start_sampled_ts = 0;
                }
                else {
                    // WIP: start_sampled_ts = average_extend_left_margin (start_ts, step);
                    // Assumption: last_average_ts (returned from db) is never going to be anything even remotely near 1970-01-01
                    start_sampled_ts = average_extend_left_margin (last_average_ts + average_step_seconds (step), step);
                }

                if (start_sampled_ts < 0) {
                    log_warning ("While timestamp in persistence is of type DATETIME, variable 'start_sampled_ts' should not be negative.");
                }
                log_info ("last_average_ts: '%" PRId64"', start_sampled_ts: '%" PRId64"'.", last_average_ts, start_sampled_ts);

                log_info ("Timestamp of last stored average: '%" PRId64"' <<  start_timestamp: '%" PRId64"'. "
                          "Therefore we need to compute averages from following sampled data interval: "
                          "<Timestamp of last stored average, end_timestamp + nut_repeat_interval> which is "
                          "<%" PRId64", %" PRId64">.",
                          last_average_ts, start_ts, start_sampled_ts, end_ts + AGENT_NUT_REPEAT_INTERVAL_SEC);
                if (!request_sampled (conn, element_id, source, start_sampled_ts,
                                      end_ts + AGENT_NUT_REPEAT_INTERVAL_SEC, samples, unit, *message_out)) {
                    log_warning ("request_sampled () failed!");
                    return;
                }
            }
            else if (start_ts <= last_average_ts && last_average_ts <= end_ts) {                        
                log_error ("persist::get_measurements_averages ('%" PRIu64"', %s, %s, %" PRId64 ", %" PRId64", ...) "
                           "returned last average timestamp: %" PRId64", that falls inside <start_timestamp, end_timestamp> "
                           "but returned map of averages is empty.",
                           element_id, source, step, start_ts, end_ts, last_average_ts);
                ymsg_set_status (*message_out, false);
                ymsg_set_errmsg (*message_out, "Internal error: Extracting data from database failed.");
                return; 
            }
            else // untreated case is end_ts < last_average_ts -> nothing to do
                log_info ("Timestamp of last stored average >> end_timestamp. End of requested interval falls into "
                          "the past that was already computed and since there are no average returned there is a gap "
                          "of sampled data.");
        }
        else {
            log_info ("Requested averages: '%" PRIu64"'", averages.size ());
            // put the stored averages into json
            for (const auto &p : averages) {
                std::string item = BIOS_WEB_AVERAGE_REPLY_JSON_DATA_ITEM_TMPL;
                std::string comp_result_str;
                utils::math::dtos (p.second, 2, comp_result_str);
                item.replace (item.find ("##VALUE##"), strlen ("##VALUE##"), comp_result_str);
                item.replace (item.find ("##TIMESTAMP##"), strlen ("##TIMESTAMP##"), std::to_string (p.first));
                if (comma_counter == 0) { 
                    ++comma_counter;
                }
                else {
                    data_str += ",\n";           
                }
                data_str += item;
            }
            // check if all requested averages were returned and if we need to compute something from sampled
            auto it = averages.end (); it--;
            int64_t last_container_ts = it->first;
            int64_t new_start;
            rv = check_completeness (last_container_ts, last_average_ts, end_ts, step, new_start);
            if (rv == -1) {
                ymsg_set_status (*message_out, false);
                ymsg_set_errmsg (*message_out, "Internal error: Extracting data from database failed.");
                return;
            }
            if (rv == 0) {
                log_info ("returned averages NOT complete. New start: '%" PRId64"'", new_start);
                start_sampled_ts = average_extend_left_margin (new_start, step);
                if (!request_sampled (conn, element_id, source, start_sampled_ts, end_ts + AGENT_NUT_REPEAT_INTERVAL_SEC,
                                      samples, unit, *message_out)) {
                    return;
                }
            }
            else
                log_info ("returned averages complete");
        }

        if (!samples.empty ()) {
            log_info ("Samples directly from db; count: '%" PRIu64"', first measurement -> timestamp: '%" PRId64"', value '%f'",
                       samples.size (), samples.cbegin ()->first, samples.cbegin ()->second);
            log_info ("Calling solve_left_margin (extended_start = '%" PRId64"').", start_sampled_ts);
            solve_left_margin (samples, start_sampled_ts);
            log_info ("Samples after solving left margin; count: '%" PRIu64"', first measurement -> timestamp: '%" PRId64"', value '%f'",
                       samples.size (), samples.cbegin ()->first, samples.cbegin ()->second);
            if (!samples.empty ()) {
                int64_t first_ts = samples.cbegin()->first;
                int64_t second_ts = average_first_since (first_ts, step);
                double comp_result;

                log_debug ("Starting computation from sampled data. first_ts: %" PRId64"\tsecond_ts: %" PRId64"\tend_ts:%" PRId64,
                           first_ts, second_ts, end_ts);
                
                // Resolve device name from element id        
                std::string device_name;
                {
                    auto ret = persist::select_device_name_from_element_id (conn, element_id, device_name);
                    if (ret.rv != 0)
                        log_error ("Could not resolve device name from element id: '%" PRId64"'. Therefore it is not possible to publish computed values on stream.", element_id);
                    else
                        log_info ("Device name resolved from element id: '%" PRId64"' is '%s'.", element_id, device_name.c_str ());
                }

                while (second_ts <= end_ts) {
                    std::string item = BIOS_WEB_AVERAGE_REPLY_JSON_DATA_ITEM_TMPL;
                    log_debug ("Calling calculate (start = '%" PRId64"', end = '%" PRId64"', type = '%s')", first_ts, second_ts, type);
                    rv = calculate (samples, first_ts, second_ts, type, comp_result);
                    if (rv == 0) {
                        if (second_ts >= start_ts) {
                            std::string comp_result_str;
                            utils::math::dtos (comp_result, 2, comp_result_str);
                            item.replace (item.find ("##VALUE##"), strlen ("##VALUE##"), comp_result_str);
                            item.replace (item.find ("##TIMESTAMP##"), strlen ("##TIMESTAMP##"), std::to_string (second_ts));
                            if (comma_counter == 0) 
                                ++comma_counter;
                            else
                                data_str += ",\n";

                            data_str += item;
                        }

                        publish_measurement (agent, device_name.c_str (), source, type, step, unit.c_str (), comp_result, second_ts);
                    }
                    else if (rv == -1) {
                        log_warning ("calculate failed ().");
                    }

                    first_ts = second_ts;
                    second_ts += average_step_seconds (step);
                }
            }  
        }
        else {
            log_info ("_samples_ empty.");
        }
                      
        json_out.replace (json_out.find ("##UNITS##"), strlen ("##UNITS##"), unit);
        json_out.replace (json_out.find ("##DATA##"), strlen ("##DATA##"), data_str);
        log_debug ("json that goes to output:\n%s", json_out.c_str ());

        ymsg_set_status (*message_out, true);
        zchunk_t *chunk = zchunk_new (json_out.c_str (), json_out.size ());
        assert (chunk);
        ymsg_set_response (*message_out, &chunk);

    }
    catch (const std::exception& e) {
        ymsg_set_status (*message_out, false);
        ymsg_set_errmsg (*message_out, "Internal error. Please check logs.");
        log_error ("std::exception caught: %s", e.what ());
    }
    catch (...) {
        ymsg_set_status (*message_out, false);
        ymsg_set_errmsg (*message_out, "Internal error. Please check logs.");
        log_error ("unknown exception caught");
    }

    FREE0 (type)
    FREE0 (step)
    FREE0 (source)
    
    return;
}

} // namespace computation::web
} // namespace computation

