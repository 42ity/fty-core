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
 \file   cm-agent-web.cc
 \brief  TODO
 \author Karol Hrdina <KarolHrdina@eaton.com>
*/

#include <utility>
#include <cmath>

#include <cxxtools/jsondeserializer.h>
#include <cxxtools/serializationerror.h>

#include "bios_agent.h"
#include "agents.h"
#include "defs.h"
#include "log.h"
#include "str_defs.h"
#include "utils.h"
#include "utils_ymsg.h"
#include "utils_ymsg++.h"

#include "measurements.h"
#include "cm-agent-utils.h"
#include "cm-agent-web.h"
#include "cleanup.h"

// TODO: Some of the functionality will be later broken down into functions
//       It doesn't make sense to do it now as non-trivial changes to logic are still ahead of us.
// TODO: Arrangement of try-catch & deallocation not ideal, but fine for the moment
int
process_web_average
(bios_agent_t *agent, ymsg_t *message_in, const char *sender, ymsg_t **message_out) {
    if (!agent || !sender || !message_in || !message_out) {
        return -1;
    }
    *message_out = ymsg_new (YMSG_REPLY);
    assert (*message_out);

    /////////////////////////////
    // Decode web average request
    int64_t start_ts = -1, end_ts = -1;
    char *type = NULL, *step = NULL, *source = NULL;
    uint64_t element_id = 0;

    int rv = bios_web_average_request_extract (message_in, &start_ts, &end_ts, &type, &step, &element_id, &source);
    if (rv != 0) {
        log_error ("bios_web_average_request_extract () failed.");
        ymsg_set_status (*message_out, false);
        ymsg_set_errmsg (*message_out, "Internal error. Please check logs.");
        return 0;       
    }

    // TODO: future step in progress, now commented
    /*    
    // First, try to request the averages
    std::map<int64_t, double> averages;
    rv = persist::get_measurements_sampled (element_id, source, start_ts, end_ts, averages, unit);
    assert (rv == 0); // TODO: check return value
    */

    // extend the requested interval
    start_ts = average_extend_left_margin (start_ts, step); // shared/utils.h 
    end_ts +=  AGENT_NUT_REPEAT_INTERVAL_SEC;


    std::string unit;
    std::map <int64_t, double> samples;
    rv = persist::get_measurements_sampled (element_id, source, start_ts, end_ts, samples, unit);
    assert (rv == 0);
    // TODO: check return value
    // TODO: remove when done testing
    printf ("samples directly from db:\n");
    for (const auto &p : samples) {
        std::cout << p.first << " => " << p.second << '\n';
    }

    try {
        log_debug ("samples map size before post-calculation: %ld\n", samples.size ());
        log_debug ("process_db_measurement_solve_left_margin");
        process_db_measurement_solve_left_margin (samples, start_ts);
        log_debug ("process_db_measurement_solve_left_margin finished");
        // TODO: remove when done testing
        printf ("samples after solving left margin:\n");
        for (const auto &p : samples) {
            std::cout << p.first << " => " << p.second << '\n';
        }

        int64_t first_ts = samples.cbegin()->first;
        int64_t second_ts = average_first_since (first_ts, step);
        double comp_result;

        std::string data_str;
        int comma_counter = 0;
        // TODO: remove when done testing
        printf ("Starting big cycle. first_ts: %ld\tsecond_ts: %ld\tend_ts:%ld\n", first_ts, second_ts, end_ts);
        while (second_ts <= end_ts) {

            std::string item = BIOS_WEB_AVERAGE_REPLY_JSON_DATA_ITEM_TMPL;
            printf ("calling process_db_measurement_calculate (%ld, %ld)\n", first_ts, second_ts); // TODO: remove when done testing
            rv = process_db_measurement_calculate (samples, first_ts, second_ts, type, comp_result);
            if (rv == 0) {
                printf ("%ld\t%f\n", second_ts, comp_result); // TODO: remove when done testing
                item.replace (item.find ("##VALUE##"), strlen ("##VALUE##"), std::to_string (comp_result));
                item.replace (item.find ("##TIMESTAMP##"), strlen ("##TIMESTAMP##"), std::to_string (second_ts));
                if (comma_counter == 0) 
                    ++comma_counter;
                else
                    data_str += ",\n";

                data_str += item;
            } else {
                log_warning ("process_db_measurement_calculate failed"); // TODO
            }
            first_ts = second_ts;
            second_ts += average_step_seconds (step);
        }

        std::string json_out(BIOS_WEB_AVERAGE_REPLY_JSON_TMPL);           
        json_out.replace (json_out.find ("##UNITS##"), strlen ("##UNITS##"), unit);
        json_out.replace (json_out.find ("##SOURCE##"), strlen ("##SOURCE##"), source);
        json_out.replace (json_out.find ("##STEP##"), strlen ("##STEP##"), step);
        json_out.replace (json_out.find ("##TYPE##"), strlen ("##TYPE##"), type);
        json_out.replace (json_out.find ("##ELEMENT_ID##"), strlen ("##ELEMENT_ID##"), std::to_string (element_id));
        json_out.replace (json_out.find ("##START_TS##"), strlen ("##START_TS##"), std::to_string (start_ts));
        json_out.replace (json_out.find ("##END_TS##"), strlen ("##END_TS##"), std::to_string (end_ts));
                      
        json_out.replace (json_out.find ("##DATA##"), strlen ("##DATA##"), data_str);
        log_debug ("json that goes to output:\n%s", json_out.c_str ());

        ymsg_set_status (*message_out, true);
        zchunk_t *chunk = zchunk_new (json_out.c_str (), json_out.size ());
        assert (chunk);
        ymsg_set_response (*message_out, &chunk);

    }
    catch (const cxxtools::SerializationError& e) {
        ymsg_set_status (*message_out, false);
        ymsg_set_errmsg (*message_out, "Internal error. Please check logs.");
        log_error ("cxxtools::SerializationError caught: %s", e.what ());
        
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

    //////////////////////
    FREE0 (type)
    FREE0 (step)
    FREE0 (source)
    
    return 0;
}



