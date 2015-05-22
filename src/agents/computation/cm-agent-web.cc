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

#include "measurement.h"
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
    ////////////////////////////////////
    // Prepare measurements read request
    _scoped_char *send_subject = NULL;
    // When user says, e.g.: "I want 8h averages starting from 23:15:.... " we need to request sampled data from an earlier period
    // in order to compute the first 8h average. This first average belongs to 24:00:00 and is computed from <16:00, 24:00).
    // Additional AGENT_NUT_REPEAT_INTERVAL_SEC seconds are deducted to complete possible leading values `missing`.
    start_ts = average_extend_left_margin (start_ts, step); // shared/utils.c 
    // we are requesting sampled data from db, therefore no topic construction (i.e. <source>.<type>_<step>) is needed.
    _scoped_ymsg_t *db_msg_send = bios_db_measurements_read_request_encode (start_ts, end_ts, element_id, source, &send_subject);
    assert (db_msg_send);  

    // TODO: remove when done testing
    std::string msg_print;
    ymsg_format (db_msg_send, msg_print);
    log_debug ("Message for persist::get_measurements ()\n%s", msg_print.c_str ()); 

    //////////////////////////////////
    // Call persistence layer directly       
    // TODO: as soon as we have a directly callable db low level api remove all of the messages
    //       but the two from `process_web_average` arguments
    _scoped_ymsg_t *db_msg_reply = ymsg_new (YMSG_REPLY);
    assert (db_msg_reply);
    _scoped_char *reply_subject = NULL;
    std::string mod_send_subject (send_subject);
    mod_send_subject.append ("<>");
    persist::get_measurements (db_msg_reply, &reply_subject, db_msg_send, send_subject);
    FREE0 (send_subject);
    FREE0 (reply_subject);
    ymsg_destroy (&db_msg_send);

    //////////////////////////////////////////////////
    // Extract payload from persistence layer response

    // TODO: Depending on how we rewrite persist::get_measurements
    //       message status error might be a meaningfull response;
    //       we might want to return the message carried
    if (!ymsg_is_ok (db_msg_reply)) {
        log_warning ("Message status = error.");
        ymsg_set_status (*message_out, false);
        ymsg_set_errmsg (*message_out, "Internal error. Please check logs.");
        return 0;
    }

    _scoped_char *json_from_db = NULL;
    rv = bios_db_measurements_read_reply_extract (db_msg_reply, &json_from_db);
    if (rv != 0 || !json_from_db || strlen (json_from_db) == 0) {
        log_error ("bios_db_measurements_read_reply_extract () failed or json empty or NULL.");
        ymsg_set_status (*message_out, false);
        ymsg_set_errmsg (*message_out, "Internal error. Please check logs.");
        return 0;       
    }
    ymsg_destroy (&db_msg_reply);   
    log_debug ("bios_db_measurements_read_reply_extract successfull.");
    printf ("json:\n%s\n", json_from_db); // TODO: remove when done testing

    //////////////////////////////////////////////
    // Do some basic pre-processing & value checks
    std::string source_db, unit_db;
    int64_t start_ts_db = -1, end_ts_db = -1;
    uint64_t element_id_db;
    std::map <int64_t, double> samples;
    // process the received json into a std::map
    rv = process_db_measurement_json_to_map (json_from_db, samples, start_ts_db, end_ts_db, element_id_db, source_db, unit_db);
    FREE0 (json_from_db);
    if (rv != 0) {
        log_error ("process_db_measurement_json_to_map() failed.");
        ymsg_set_status (*message_out, false);
        ymsg_set_errmsg (*message_out, "Internal error. Please check logs.");
        return 0;
    }

    // sanity check
    if ((element_id != element_id_db) || (end_ts != end_ts_db)) {
        log_error ("Values returned from persistence DO NOT match the requested.");
        ymsg_set_status (*message_out, false);
        ymsg_set_errmsg (*message_out, "Internal error. Please check logs."); 
        return 0;
    }
    //////////////////////
    try {
        log_debug ("samples map size before post-calculation: %ld\n", samples.size ());
        log_debug ("process_db_measurement_solve_left_margin");
        process_db_measurement_solve_left_margin (samples, start_ts_db);
        log_debug ("process_db_measurement_solve_left_margin finished");
        // TODO: remove when done testing
        for (const auto &p : samples) {
            std::cout << p.first << " => " << p.second << '\n';
        }

        int64_t first_ts = samples.cbegin()->first;
        int64_t second_ts = average_first_since (first_ts, step);
        double comp_result;

        std::string data_str;
        int comma_counter = 0;

        printf ("Starting big cycle. first_ts: %ld\tsecond_ts: %ld\tend_ts:%ld\n", first_ts, second_ts, end_ts_db);
        while (second_ts <= end_ts_db) {

            std::string item = BIOS_WEB_AVERAGE_REPLY_JSON_DATA_ITEM_TMPL;
            printf ("calling process_db_measurement_calculate (%ld, %ld)\n", first_ts, second_ts);
            rv = process_db_measurement_calculate (samples, first_ts, second_ts, type, comp_result);
            if (rv == 0) {
                printf ("%ld\t%f\n", second_ts, comp_result);
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

        // TODO
        std::string json_out(BIOS_WEB_AVERAGE_REPLY_JSON_TMPL);           
        json_out.replace (json_out.find ("##UNITS##"), strlen ("##UNITS##"), unit_db);
        json_out.replace (json_out.find ("##SOURCE##"), strlen ("##SOURCE##"), source);
        json_out.replace (json_out.find ("##STEP##"), strlen ("##STEP##"), step);
        json_out.replace (json_out.find ("##TYPE##"), strlen ("##TYPE##"), type);
        json_out.replace (json_out.find ("##ELEMENT_ID##"), strlen ("##ELEMENT_ID##"), std::to_string (element_id_db));
        json_out.replace (json_out.find ("##START_TS##"), strlen ("##START_TS##"), std::to_string (start_ts));
        json_out.replace (json_out.find ("##END_TS##"), strlen ("##END_TS##"), std::to_string (end_ts));
                      
        json_out.replace (json_out.find ("##DATA##"), strlen ("##DATA##"), data_str);
        log_debug ("json:\n%s", json_out.c_str ());

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



