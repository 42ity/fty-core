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
#include "utils.h"
#include "utils_ymsg.h"
#include "utils_ymsg++.h"

#include "cm-agent-utils.h"
#include "cleanup.h"

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

void
process_db_measurement_solve_left_margin
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
    if (it != samples.end ())
        log_debug ("Lower bound returned timestamp: %" PRId64", value: %f", it->first, it->second);
    else
        log_debug ("Lower bound returned samples.end ()");
    if (it == samples.end () || it->first >= start) {
        log_debug ("Lower bound == samples.end () || lower bound >= start");
        samples.erase (samples.cbegin (), it);
        return;
    }

    while (it->first < start) { ++it; }
    std::map <int64_t, double>::const_iterator i;
    bool inserted;
    std::tie (i, inserted) = samples.emplace (std::make_pair (extended_start + AGENT_NUT_REPEAT_INTERVAL_SEC, (--it)->second));
    if (inserted)
        log_debug ("emplace ok");
    else 
        log_debug ("did NOT emplace");

    //cut the beginning
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
    assert (type);
    if (start >= end || samples.empty ())
        return -1;

    // TODO: Remove when done testing
    printf ("Inside _calculate_arithmetic_mean (%ld, %ld)\n", start, end);
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
            printf ("it2 == samples.end ()\n"); // TODO: Remove when done testing
            weight = 1;    
        }
        else if (it2->first >= end) {                       
            if (it2->first != end && (it2->first - it1->first) <= AGENT_NUT_REPEAT_INTERVAL_SEC) {
                weight = sample_weight (it1->first, end);
                samples.emplace (std::make_pair (end, it1->second));
                printf ("emplacing value '%ld' with timestamp '%f'\n", end, it1->second); // TODO: Remove when done testing
            } else {
                weight = 1;
            }
            printf ("it1->first: %ld --> end: %ld\tweight: %ld\tit1->second: %f\n", it1->first, end, weight, it1->second);

        }
        else { // it2->first < end 
            weight = sample_weight (it1->first, it2->first);
            printf ("it1->first: %ld --> it2->first: %ld\tweight: %ld\tit1->second: %f\n", it1->first, it2->first, weight, it1->second); // TODO: Remove when done testing
        }

        if (weight <= 0) {
            log_error ("Weight can not be negative!");
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

    int64_t block = 0;

    int64_t step_sec = average_step_seconds (step);
    if (end_timestamp - last_average_timestamp < step_sec) {
        return 1;
    }
    // end_timestamp - last_average_timestamp >= step_sec
    if (last_average_timestamp <= last_container_timestamp) {
        new_start = last_container_timestamp + step_sec;
        return 0;
    }
    return 1;
}


