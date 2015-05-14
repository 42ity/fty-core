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
 \file   cm-agent-utils.cc
 \brief  TODO
 \author Karol Hrdina <KarolHrdina@eaton.com>
*/
#include <utility>
#include <cmath>

#include <cxxtools/jsondeserializer.h>
#include <cxxtools/serializationerror.h>

#include "defs.h"
#include "log.h"
#include "utils_ymsg.h"
#include "utils_ymsg++.h"

#include "cm-agent-utils.h"
#include "cleanup.h"

int 
replyto_err 
(bios_agent_t *agent, ymsg_t **original, const char *sender, const char *error_message, const char *subject) {
    if (!original || !*original)
        return -1;
    if (!agent || !sender || !error_message || !subject) {
        ymsg_destroy (original);
        return -1;
    }

    _scoped_ymsg_t *msg_reply = ymsg_new (YMSG_REPLY);
    assert (msg_reply);
    ymsg_set_status (msg_reply, false);
    ymsg_set_errmsg (msg_reply, error_message);
    std::string formatted_message;
    ymsg_format (msg_reply, formatted_message);
    int rv = bios_agent_replyto (agent, sender, subject, &msg_reply, *original); // msg_reply is destroyed
    ymsg_destroy (original); // original is destroyed
    if (rv != 0) {
        log_critical ("bios_agent_replyto (\"%s\", \"%s\") failed.", sender, subject);
    }
    else {
        log_debug ("ACTION. Error message sent to %s with subject %s:\n%s", sender, subject, formatted_message.c_str ());
    }
    return 0;
}

int
process_db_measurement_json_to_map
(const char *json, std::map <int64_t, double>& samples, int64_t& start_timestamp, int64_t& end_timestamp, uint64_t& element_id, std::string& source, std::string& unit) {
    samples.clear ();
    if (!json)
        return -1;
    try {
        // process the received json into a std::map
        std::stringstream input (json, std::ios_base::in);

        cxxtools::SerializationInfo si;
        cxxtools::JsonDeserializer deserializer (input);
   
        std::string start_ts_str, end_ts_str, element_id_str;
        deserializer.deserialize (si);
        
        if (si.category () != cxxtools::SerializationInfo::Category::Object) {
            throw std::invalid_argument ("Root type is not object.");
        }

        si.getMember ("start_ts") >>= start_ts_str;
        if (start_ts_str.empty ()) {
            throw std::invalid_argument ("Field 'start_ts' is empty.");
        }
        si.getMember ("end_ts") >>= end_ts_str;
        if (end_ts_str.empty ()) {
            throw std::invalid_argument ("Field 'end_ts' is empty.");
        }
        si.getMember ("source") >>= source;
        if (source.empty ()) {
            throw std::invalid_argument ("Field 'source' is empty.");
        }
        si.getMember ("unit") >>= unit;
        if (unit.empty ()) {
            throw std::invalid_argument ("Field 'unit' is empty.");
        }
        si.getMember ("element_id") >>= element_id_str;
        if (element_id_str.empty ()) {
            throw std::invalid_argument ("Field 'element_id' is empty.");
        }
        // throws std::invalid_argument, std::out_of_range
        element_id = std::stoull (element_id_str);
        start_timestamp = std::stoll (start_ts_str);
        end_timestamp = std::stoll (end_ts_str);

        log_debug("start_ts: '%ld'  end_ts: '%ld'  source: '%s'  unit: '%s'  element_id: '%ld'",
                  start_timestamp, end_timestamp, source.c_str (), unit.c_str (), element_id);

        auto data_array = si.getMember("data");
        if (si.getMember("data").category () != cxxtools::SerializationInfo::Category::Array) {
            throw std::invalid_argument ("Value of key 'data' is not an array.");
        }
        if (si.getMember("data").memberCount () <= 0) {
            throw std::invalid_argument ("Value of key 'data' is an empty array.");
        }

        for (auto it = data_array.begin (), it_end = data_array.end (); it != it_end; ++it) {
            if (it->category() != cxxtools::SerializationInfo::Category::Object) {
                throw std::invalid_argument ("Item of array data is not an object.");
            }
            std::string value, scale, timestamp;                    
            it->getMember ("value") >>= value;
            it->getMember ("scale") >>= scale;
            it->getMember ("timestamp") >>= timestamp;                    
            double comp_value = std::stol (value)  * std::pow (10, std::stoi (scale));
            // TODO: Remove when done testing
            printf ("value: %s\tscale: %s\ttimestamp: %s\tcomputed_value: %f\n", value.c_str(), scale.c_str(), timestamp.c_str(), comp_value);
            samples.emplace (std::make_pair ( std::stoll (timestamp), comp_value));
        }        
    }
    catch (const cxxtools::SerializationError& e) {
        log_error ("cxxtools::SerializationError caught: %s", e.what ());
        return -1;        
    }
    catch (const std::exception& e) {
        log_error ("std::exception caught: %s", e.what ());
        return -1;
    }
    catch (...) {
        log_error ("unknown exception caught");
        return -1;
    }  
    return 0;
}

void
process_db_measurement_solve_left_margin
(std::map <int64_t, double>& samples, int64_t extended_start) {
    if (samples.empty () || extended_start < 0)
        return;
    int64_t start = extended_start + AGENT_NUT_REPEAT_INTERVAL_SEC;
    if (samples.cbegin()->first >= start)
        return;
    auto it = samples.find (start);
    if (it != samples.end ()) {
        samples.erase (samples.cbegin(), it);
        return;
    }

    it = samples.lower_bound (extended_start);
    if (it == samples.end () || it->first >= start) {
        samples.erase (samples.cbegin (), it);
        return;
    }

    while (it->first < start) { ++it; }
    std::map <int64_t, double>::const_iterator i;
    bool inserted;
    std::tie (i, inserted) = samples.emplace (std::make_pair (extended_start + AGENT_NUT_REPEAT_INTERVAL_SEC, (--it)->second));
    
    //cut the beginnning
    samples.erase (samples.cbegin (), i);
}

int
process_db_measurement_calculate
(std::map <int64_t, double>& samples, int64_t start, int64_t end, const char *type, double& result) {
    if (!type || start >= end || samples.empty () || start < 0 || end < 0 )
        return -1;

    auto it1 = samples.lower_bound (start);
    if (it1->first >= end) // requested interval is empty
        return 1;
    // TODO: add check from utils.h for validity of type

    if (strcmp (type, "min") == 0) {
        double minimum = it1->second;
        while (it1 != samples.end () && it1->first < end) {
            if (it1->second < minimum)
                minimum = it1->second;
            ++it1;
        }
        result = minimum;
    }
    else if (strcmp (type, "max") == 0) {
        double maximum = it1->second;
        while (it1 != samples.end () && it1->first < end) {
            if (it1->second > maximum)
                maximum = it1->second;
            ++it1;
        }
        result = maximum;
    }
    else if (strcmp (type, "arithmetic_mean") == 0) {
        return process_db_measurement_calculate_arithmetic_mean (samples, start, end, type, result);
    }
    else
        return -1;
    if (it1 != samples.end () && it1->first != end) {
        auto it2 = it1; --it2;
        if (it1->first - it2->first <= AGENT_NUT_REPEAT_INTERVAL_SEC) {
            samples.emplace (std::make_pair (end, it2->second));
        }
    }
    return 0;
}

int
process_db_measurement_calculate_arithmetic_mean
(std::map <int64_t, double>& samples, int64_t start, int64_t end, const char *type, double& result) {
    if (!type || start >= end || samples.empty ())
        return -1;

    auto it1 = samples.lower_bound (start);
    if (it1 == samples.end () || it1->first >= end) // requested interval is empty
        return 1;
    if (samples.size () == 1) {
        result = it1->second;
        return 0; 
    }

    uint32_t counter = 0;
    int64_t sum = 0;

    while (it1 != samples.end () && it1->first < end) {
        auto it2 = it1; ++it2;
        uint32_t count = 0;
        int64_t difference = 0;
        if (it2 == samples.end () || it2->first >= end) {
            difference = end - it1->first;
            if (difference > AGENT_NUT_REPEAT_INTERVAL_SEC)
                count = 1;
            else {
                count = (difference / AGENT_NUT_SAMPLING_INTERVAL_SEC);
                if (count == 0)
                    count = 1;
            }
            counter += count;
            sum += it1->second * count;
            break;
        }
        difference = it2->first - it1->first;
        if (difference > AGENT_NUT_REPEAT_INTERVAL_SEC)
            count = 1;
        else
            count = (it2->first - it1->first) / AGENT_NUT_SAMPLING_INTERVAL_SEC;
        if (count == 0)
            count = 1;
        counter += count;        
        sum += it1->second * count;
        ++it1;        
    }
    result = sum / counter;
    if (it1 != samples.end () && it1->first != end) {
        auto it2 = it1; --it2;
        if (it1->first - it2->first <= AGENT_NUT_REPEAT_INTERVAL_SEC) {
            samples.emplace (std::make_pair (end, it2->second));
        }
    }
    return 0;
}
