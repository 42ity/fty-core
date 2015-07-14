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
 \file   cm-utils.cc
 \brief  Implementation of utility functions and helpers for computation module
 \author Karol Hrdina <KarolHrdina@eaton.com>
*/

#include <utility>
#include <cmath>

#include <cxxtools/jsondeserializer.h>
#include <cxxtools/serializationerror.h>

#include "defs.h"
#include "str_defs.h"
#include "log.h"
#include "utils.h"
#include "utils_ymsg.h"
#include "utils_ymsg++.h"

#include "db/measurements.h"
#include "cm-utils.h"
#include "cleanup.h"

namespace computation {

namespace web {

int64_t
sample_weight (int64_t begin, int64_t end) {
    if (begin >= end) {
        return -1;
    }
    int64_t difference = end - begin;
    if (difference > AGENT_NUT_REPEAT_INTERVAL_SEC) {
        return 1;
    }
    return difference;
}

void
solve_left_margin
(std::map <int64_t, double>& samples, int64_t extended_start) {
    if (samples.empty ()) {
        log_debug ("Samples empty.");
        return;
    }
    int64_t start = extended_start + AGENT_NUT_REPEAT_INTERVAL_SEC;
    log_debug ("Start: %" PRId64, start);
    if (samples.cbegin()->first >= start) {
        log_debug ("Nothing to solve. First item in samples: %" PRId64", >= start: %" PRId64, samples.cbegin()->first, start);
        return;
    }
    auto it = samples.find (start);
    if (it != samples.end ()) {
        log_debug ("Value exactly on start: %" PRId64" exists, value = %f", it->first, it->second);
        samples.erase (samples.cbegin(), it);
        return;
    }

    it = samples.lower_bound (extended_start);
    if (it != samples.end ()) {
        log_debug ("Lower bound returned timestamp: %" PRId64", value: %f", it->first, it->second);
    }
    else if (it == samples.end () || it->first >= start) {
        log_debug ("Lower bound == samples.end () || lower bound >= start");
        samples.erase (samples.cbegin (), it);
        return;
    }

    while (it->first < start) {
        ++it;
        if (it == samples.end ()) {
            samples.clear ();
            return;
        }
    }
    auto prev = it; --prev;

    if (it->first - prev->first <= AGENT_NUT_REPEAT_INTERVAL_SEC) {
        std::map <int64_t, double>::const_iterator i;
        bool inserted;
        std::tie (i, inserted) = samples.emplace (std::make_pair (extended_start + AGENT_NUT_REPEAT_INTERVAL_SEC, prev->second));
        if (inserted)
            log_debug ("emplace ok");
        else 
            log_debug ("did NOT emplace");

        //cut the beginning
        samples.erase (samples.cbegin (), i);
    } else {
        samples.erase (samples.cbegin (), it);
    }
}

int
calculate
(std::map <int64_t, double>& samples, int64_t start, int64_t end, const char *type, double& result) {
    assert (type);
    assert (is_average_type_supported (type));
    if (start >= end) {
        log_debug ("'start' timestamp >= 'end' timestamp");
        return -1;
    }
    if (samples.empty ()) {
        log_debug ("Supplied samples map is empty.");
        return 1; 
    }
    
    auto it1 = samples.lower_bound (start);
    if (it1 == samples.end ()) {
        log_debug ("There is no sample >= 'start' timestamp.");
        return 1;
    }

    if (it1->first >= end) {
        log_debug ("First sample >= 'start' timestamp is >= 'end'. Requested interval empty.");
        return 1;
    }

    if (strcmp (type, AVG_TYPES[1]) == 0) { // min
        double minimum = it1->second;
        while (it1 != samples.end () && it1->first < end) {
            if (it1->second < minimum)
                minimum = it1->second;
            ++it1;
        }
        result = minimum;
    }
    else if (strcmp (type, AVG_TYPES[2]) == 0) { // max
        double maximum = it1->second;
        while (it1 != samples.end () && it1->first < end) {
            if (it1->second > maximum)
                maximum = it1->second;
            ++it1;
        }
        result = maximum;
    }
    else if (strcmp (type, AVG_TYPES[0]) == 0) { // arithmetic_mean
        return calculate_arithmetic_mean (samples, start, end, result);
    }
    else {
        log_debug ("Call `is_average_type_supported ('%s')` returned true; there is probably "
                   "no if-branch to handle the supported average type.", type);
        return -1;
    }

    if (it1 != samples.end () && it1->first != end) {
        auto it2 = std::prev (it1); // TODO: remove --it2;
        if (it1->first - it2->first <= AGENT_NUT_REPEAT_INTERVAL_SEC) {
            log_debug ("'End' timestamp: '%" PRId64"'; first value _before_ - timestamp: '%" PRId64"', value: '%f'; "
                       " first value _after_ - timestamp: '%" PRId64"', value: '%f'; "
                       " Emplacing at 'end' timestamp value '%f'.",
                       end, it2->first, it2->second, it1->first, it1->second, it2->second);
            samples.emplace (std::make_pair (end, it2->second));
        }
    }
    return 0;
}

int
calculate_arithmetic_mean
(std::map <int64_t, double>& samples, int64_t start, int64_t end, double& result) {
    if (start >= end || samples.empty ())
        return -1;
    
    log_debug ("Inside _calculate_arithmetic_mean (%ld, %ld)\n", start, end); // TODO: remove when done testing
    auto it1 = samples.lower_bound (start);
    if (it1 == samples.end () || it1->first >= end) { // requested interval is empty
        log_debug ("requested interval empty");
        return 1;
    }
    if (samples.size () == 1) {
        result = it1->second;
        return 0; 
    }

    uint32_t counter = 0;
    int64_t sum = 0;

    while (it1 != samples.end () && it1->first < end) {
        auto it2 = it1; ++it2;
        int64_t weight = 0;

        if (it2 == samples.end ()) {
            log_debug ("it2 == samples.end ()\n"); // TODO: Remove when done testing
            weight = 1;    
        }
        else if (it2->first >= end) {
            // WIP (leave until tested): if (it2->first != end && (it2->first - it1->first) <= AGENT_NUT_REPEAT_INTERVAL_SEC) { 
            if ((it2->first - it1->first) <= AGENT_NUT_REPEAT_INTERVAL_SEC) {
                weight = sample_weight (it1->first, end);
                samples.emplace (std::make_pair (end, it1->second));
                log_debug ("emplacing value '%ld' with timestamp '%f'\n", end, it1->second); // TODO: Remove when done testing
            } else {
                weight = 1;
            }
            log_debug ("it1->first: %ld --> end: %ld\tweight: %ld\tit1->second: %f\n", it1->first, end, weight, it1->second); // TODO: Remove when done testing

        }
        else { // it2->first < end 
            weight = sample_weight (it1->first, it2->first);
            log_debug ("it1->first: %ld --> it2->first: %ld\tweight: %ld\tit1->second: %f\n", it1->first, it2->first, weight, it1->second); // TODO: Remove when done testing
        }

        if (weight <= 0) {
            log_error ("Weight can not be negative or zero! Begin timestamp >= end timestamp.");
            return -1;
        }
        counter += weight;
        sum += it1->second * weight;
        ++it1;
    }

    result = (double) sum / counter;
    return 0;
}

int
check_completeness
(int64_t last_container_timestamp, int64_t last_average_timestamp, int64_t end_timestamp, const char *step, int64_t& new_start) {
    assert (step);
    assert (is_average_step_supported (step));


    int64_t step_sec = average_step_seconds (step);
    log_debug ("last container: %" PRId64"\t last average: %" PRId64"\tend: %" PRId64"\tstep: '%s'\tstep seconds: %" PRId64,
                last_container_timestamp, last_average_timestamp, end_timestamp, step, step_sec);

    if (last_average_timestamp < last_container_timestamp) {
        log_error ("last average timestamp < last container timestamp");
        return -1;
    }

    if (end_timestamp - last_container_timestamp < step_sec) {
        return 1;
    }
    // end_timestamp - last_container_timestamp >= step_sec
    if (last_average_timestamp > end_timestamp ) {
        return 1;
    }
    else { // last_average_timestamp <= end_timestamp
        if (last_average_timestamp == last_container_timestamp) {
            new_start = last_container_timestamp + step_sec;
            return 0;
        }
        else { 
            log_error ("last container timestamp < last average timestamp <= end timestamp");
            return -1;
        }
    }
    return 1;
}

int
request_averages
(tntdb::Connection& conn, int64_t element_id, const char *source, const char *type, const char *step, int64_t start_timestamp, int64_t end_timestamp,
 std::map<int64_t, double>& averages, std::string& unit, int64_t& last_average_timestamp, ymsg_t *message_out) {
    assert (source);
    assert (type);
    assert (step);
    assert (message_out);

    int return_value = 0;
    std::string message_str;
    log_debug ("Requesting averages element_id: '%" PRId64"', source: '%s', type: '%s', step: '%s', "
               " start_timestamp: '%" PRId64"', end_timestamp: '%" PRId64"'",
               element_id, source, type, step, start_timestamp, end_timestamp );
    auto ret = persist::select_measurements_averages
    (conn, element_id, source, type, step, start_timestamp, end_timestamp, averages, unit, last_average_timestamp);
    switch (ret.rv) {
        case 0:
        {
            return_value = 1;
            log_debug ("Ok. Last average timestamp: '%" PRId64"'", last_average_timestamp);
            break;
        }
        case 1:
        {
            log_info ("Element id: %" PRIu64" does not exist in persistence.", element_id);
            ymsg_set_status (message_out, false);
            message_str.assign ("Element id '").append (std::to_string (element_id)).append ("' does not exist.");
            ymsg_set_errmsg (message_out, message_str.c_str ());
            return_value = 0;
            break;
        }
        case 2:
        {
            log_info ("Topic does not exist for element id: '%" PRIu64"', source: '%s' and step: '%s' or element not in discovered devices.", element_id, source, step);
            return_value = 1;
            break;
        }
        case -1:
        default:
        {
            log_error ("persist::select_measurements_averages ('%" PRIu64"', %s, %s, %s, %" PRId64 ", %" PRId64", ...) failed",
                    element_id, source, type, step, start_timestamp, end_timestamp);
            ymsg_set_status (message_out, false);
            ymsg_set_errmsg (message_out, "Internal error: Extracting data from database failed.");
            return_value = 0;
            break;
        }
    }
    return return_value;
}

int
request_sampled
(tntdb::Connection& conn, int64_t element_id, const char *topic, int64_t start_timestamp, int64_t end_timestamp,
  std::map<int64_t, double>& samples, std::string& unit, ymsg_t *message_out) {
    assert (topic);
    assert (message_out);

    int return_value = 0;
    std::string message_str;
    log_debug ("Requesting samples element_id: '%" PRId64"', topic: '%s', start_timestamp: '%" PRId64"', end_timestamp: '%" PRId64"'",
               element_id, topic, start_timestamp, end_timestamp );
    auto ret = persist::select_measurements_sampled (conn, element_id, topic, start_timestamp, end_timestamp, samples, unit);
    switch (ret.rv) {
        case 0:
        {
            return_value = 1;
            break;
        }
        case 1:
        {
            log_info ("Element id: %" PRIu64" does not exist in persistence.", element_id);
            ymsg_set_status (message_out, false);
            message_str.assign ("Element id '").append (std::to_string (element_id)).append ("' does not exist.");
            ymsg_set_errmsg (message_out, message_str.c_str ());
            return_value = 0;
            break;
        }
        case 2:
        {
            log_info ("Topic '%s' does not exist for element id: '%" PRIu64"'", topic, element_id);
            ymsg_set_status (message_out, false);
            message_str.assign ("Topic '").append (topic).append ("' does not exist for element id '").append (std::to_string (element_id)).append ("'");
            ymsg_set_errmsg (message_out, message_str.c_str ());
            return_value = 0;
            break;
        }
        case -1:
        default:
        {
            log_error ("persist::select_measurements_sampled ('%" PRIu64"', %s, %" PRId64 ", %" PRId64", ...) failed",
                    element_id, topic, start_timestamp, end_timestamp);
            ymsg_set_status (message_out, false);
            ymsg_set_errmsg (message_out, "Internal error: Extracting data from database failed.");
            return_value = 0;
            break;
        }
    }
    return return_value;  
}




} // namespace web

void
publish_measurement
(bios_agent_t *agent, const char *device_name, const char *source, const char *type, const char *step, const char *unit, double value, int64_t timestamp) {
    assert (device_name);
    assert (source);
    assert (type);
    assert (step);
    assert (unit);

    if (strlen (device_name) == 0) {
        log_info ("Device name empty, measurement not published.");
        return;
    }
    std::string quantity;
    quantity.assign (source).append ("_").append (type).append ("_").append (step);
    log_debug ("quantity: '%s'", quantity.c_str ());

    std::string topic;
    topic.assign ("measurement.").append (quantity).append ("@").append (device_name);
    log_debug ("topic: '%s'", topic.c_str ());

    // Note: precision is currently hardcoded for 2 floating point places


    _scoped_ymsg_t *published_measurement =
        bios_measurement_encode (device_name, quantity.c_str(), unit, (int32_t) std::round (value * 100), -2, timestamp); // TODO: propagae this upwards....
    if (!published_measurement) {
        log_critical (
        "bios_measurement_encode (device = '%s', source = '%s', type = '%s', step = '%s', value = '%f', timestamp = '%" PRId64"') failed.",
        device_name, source, type, step, value, timestamp);
        return;
    }
    std::string formatted_msg;
    ymsg_format (published_measurement, formatted_msg);
    int rv = bios_agent_send (agent, topic.c_str (), &published_measurement); // published_measurement destroyed 
    if (rv == 0) {
        log_debug ("Publishing message on stream '%s' with subject '%s':\n%s", bios_get_stream_main (), topic.c_str (), formatted_msg.c_str());
    } else {
        log_critical ("bios_agent_send (subject = '%s') failed.", topic.c_str ());
    } 
}

} // namespace computation

