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



static int
process_db_measurement
(ymsg_t **message_in_p, ymsg_t *message_out, int64_t orig_start_ts, int64_t orig_end_ts, uint64_t orig_element_id, const char *orig_type, const char *orig_step) {
    if (!message_in_p || !*message_in_p || !message_out)
        return -1;

    ymsg_t *message_in = *message_in_p;    
    std::string msg_print;
    if (ymsg_id (message_in) == YMSG_SEND) {
        log_error ("YMSG_REPLY expected");
        return -1;
    }

    int rv = 0;
    
    char *json_in = NULL;
    rv = bios_db_measurements_read_reply_extract (message_in, &json_in);
    if (rv != 0 || !json_in || strlen (json_in) == 0) {
        log_error ("bios_db_measurements_read_reply_extract () failed or json empty or NULL.");
        ymsg_destroy (message_in_p);

        ymsg_set_status (message_out, false);
        ymsg_set_errmsg (message_out, "Internal error. Please check logs.");

        return 0;       
    }
    ymsg_destroy (message_in_p);
    log_debug ("bios_db_measurements_read_reply_extract successfull.");
    printf ("json:\n%s\n", json_in); // TODO: remove when done testing
  
    std::string source, unit;
    int64_t start_ts = -1, end_ts = -1;
    uint64_t element_id;
    std::map <int64_t, double> samples;
    // process the received json into a std::map
    rv = process_db_measurement_json_to_map (json_in, samples, start_ts, end_ts, element_id, source, unit);
    FREE0 (json_in);
    if (rv == -1) {
        log_error ("process_db_measurement_json_to_map() failed.");
        ymsg_set_status (message_out, false);
        ymsg_set_errmsg (message_out, "Internal error. Please check logs.");
        return 0;
    }

    try {

        log_debug ("samples map size before post-calculation: %ld\n", samples.size ());                            


// TODO: rewrite the sanity check up one level
/*
        int64_t orig_start_ts, orig_end_ts;
        uint64_t orig_element_id;
        char *orig_type = NULL, *orig_step = NULL, *orig_source = NULL;

        rv = bios_web_average_request_extract (msg_orig, &orig_start_ts, &orig_end_ts, &orig_type, &orig_step, &orig_element_id, &orig_source);
        if (rv != 0) { // this should never happen; the stored message had already been successfully decoded once
            // log error and send to to_address
            log_critical ("bios_web_average_request_extract () failed.");
            rv = replyto_err (agent, &msg_orig, to_address.c_str (), "Internal error. Check logs.", ""); // msg_orig is destroyed;
                                                                                                    // TODO: when subject added to output args...
            assert (rv == 0);
            return 0;
        }
*/        
        // sanity check
        // this also should never happen; db-ng should not alter the requested values
        // TODO: include orig_source
        if ((element_id != orig_element_id) ||
            (end_ts != orig_end_ts) ) {
            // log and send reply to to_address
            log_error ("requested and returned values don't match"); //TODO
            ymsg_set_status (message_out, false);
            ymsg_set_errmsg (message_out, "Internal error. Please check logs."); 

            return 0;
        }

        log_debug ("process_db_measurement_solve_left_margin");
        process_db_measurement_solve_left_margin (samples, start_ts);
        log_debug ("process_db_measurement_solve_left_margin finished");
        // TODO: remove when done testing
        for (const auto &p : samples) {
            std::cout << p.first << " => " << p.second << '\n';
        }

        int64_t first_ts = samples.cbegin()->first;
        int64_t second_ts = average_first_since (first_ts, orig_step);
        double comp_result;

        std::string data_str;
        int comma_counter = 0;

        log_debug ("pre while-cycle"); 
        log_debug ("second_ts: %ld\tend_ts:%ld", second_ts, end_ts);
        while (second_ts <= end_ts) {

            std::string item = BIOS_WEB_AVERAGE_REPLY_JSON_DATA_ITEM_TMPL;
            rv = process_db_measurement_calculate
                (samples, first_ts, second_ts, orig_type, comp_result);
            if (rv == 0) {
                printf ("%ld\t%f\n", second_ts, comp_result);
                item.replace (item.find ("##VALUE##"), strlen ("##VALUE##"), std::to_string (comp_result));
                item.replace (item.find ("##TIMESTAMP##"), strlen ("##TIMESTAMP##"), std::to_string (second_ts));
                if (comma_counter == 0) 
                    ++comma_counter;
                else
                    data_str += ",\n";

                data_str += item;
            }
            int64_t tmp = second_ts;
            first_ts = second_ts;
            second_ts += average_step_seconds (orig_step);
            log_debug ("second_ts: %ld\tend_ts:%ld", second_ts, end_ts);
        }
        log_debug ("post while-cycle"); 

        // TODO
        std::string json_out(BIOS_WEB_AVERAGE_REPLY_JSON_TMPL);           
        json_out.replace (json_out.find ("##UNITS##"), strlen ("##UNITS##"), unit);
        json_out.replace (json_out.find ("##SOURCE##"), strlen ("##SOURCE##"), source);
        json_out.replace (json_out.find ("##STEP##"), strlen ("##STEP##"), orig_step);
        json_out.replace (json_out.find ("##TYPE##"), strlen ("##TYPE##"), orig_type);
        json_out.replace (json_out.find ("##ELEMENT_ID##"), strlen ("##ELEMENT_ID##"), std::to_string (element_id));
        json_out.replace (json_out.find ("##START_TS##"), strlen ("##START_TS##"), std::to_string (orig_start_ts));
        json_out.replace (json_out.find ("##END_TS##"), strlen ("##END_TS##"), std::to_string (orig_end_ts));
                      
        json_out.replace (json_out.find ("##DATA##"), strlen ("##DATA##"), data_str);
        log_debug ("json:\n%s", json_out.c_str ());
        


        ymsg_set_status (message_out, true);
        zchunk_t *chunk = zchunk_new (json_out.c_str (), json_out.size ());
        assert (chunk);
        ymsg_set_response (message_out, &chunk);

    }
    catch (const cxxtools::SerializationError& e) {
        ymsg_set_status (message_out, false);
        ymsg_set_errmsg (message_out, "Internal error. Please check logs.");
        ymsg_destroy (message_in_p);
        log_error ("cxxtools::SerializationError caught: %s", e.what ());
        
    }
    catch (const std::exception& e) {
        ymsg_set_status (message_out, false);
        ymsg_set_errmsg (message_out, "Internal error. Please check logs.");
        ymsg_destroy (message_in_p);
        log_error ("std::exception caught: %s", e.what ());
    }
    catch (...) {
        ymsg_set_status (message_out, false);
        ymsg_set_errmsg (message_out, "Internal error. Please check logs.");
        ymsg_destroy (message_in_p);
        log_error ("unknown exception caught");
    }
    return 0;
}

int
process_web_average
(bios_agent_t *agent, ymsg_t *message_in, const char *sender, ymsg_t **message_out) {
    if (!agent || !sender || !message_in || !message_out)
        return -1;

    // average request decode variables
    int64_t start_ts = -1, end_ts = -1;
    char *type = NULL, *step = NULL, *source = NULL;
    uint64_t element_id = 0;

    int rv = bios_web_average_request_extract (message_in, &start_ts, &end_ts, &type, &step, &element_id, &source);
    if (rv != 0) {
        log_error ("bios_web_average_request_extract () failed.");

        ymsg_t *msg_reply = ymsg_new (YMSG_REPLY);
        assert (msg_reply);
        ymsg_set_status (msg_reply, false);
        ymsg_set_errmsg (msg_reply, "Internal error. Please check logs."); // TODO: maybe rewrite the message

        *message_out = msg_reply;
        return 0;       
    }
    // Prepare measurements read request
    _scoped_char *send_subject = NULL;
    // When user says, e.g.: "I want 8h averages starting from 23:15:.... " we need to request sampled data from an earlier period
    // in order to compute the first 8h average. This first average belongs to 24:00:00 and is computed from <16:00, 24:00).
    // Additional AGENT_NUT_REPEAT_INTERVAL_SEC seconds are deducted to complete possible leading values `missing`.
    start_ts = average_extend_left_margin (start_ts, step); // shared/utils.c 
    // we are requesting sampled data from db, therefore no topic construction (i.e. <source>.<type>_<step>) is needed.
    _scoped_ymsg_t *db_msg_send = bios_db_measurements_read_request_encode (start_ts, end_ts, element_id, source, &send_subject);
    assert (db_msg_send);  

    // TODO
    std::string msg_print;
    ymsg_format (db_msg_send, msg_print);

    // INPROGRESS: delete
    // send the request to db-ng
    //    rv = bios_agent_sendto (agent, BIOS_AGENT_NAME_DB_MEASUREMENT, send_subject, &msg_send); // msg_send is destroyed

    // call directly       
    _scoped_ymsg_t *db_msg_reply = ymsg_new (YMSG_REPLY);
    assert (db_msg_reply);
    _scoped_char *reply_subject = NULL;
    persist::get_measurements (db_msg_reply, &reply_subject, db_msg_send, send_subject);

   
    FREE0 (send_subject);
    FREE0 (reply_subject);
    ymsg_destroy (&db_msg_send);

    if (!ymsg_is_ok (db_msg_reply)) {
        log_warning ("Message status = error.");
        ymsg_t *msg_reply = ymsg_new (YMSG_REPLY);
        assert (msg_reply);
        ymsg_set_status (msg_reply, false);
        ymsg_set_errmsg (msg_reply, "Internal error. Please check logs."); // TODO: maybe rewrite the message

        *message_out = msg_reply;

        return 0;
    }

    ymsg_t *msg_tmp = ymsg_new (YMSG_REPLY);
    assert (msg_tmp);
    rv = process_db_measurement (&db_msg_reply, msg_tmp, start_ts, end_ts, element_id, type, step);
    FREE0 (type)
    FREE0 (step)
    FREE0 (source)

    assert (rv == 0);
    log_info ("process_db_measurement finished.");
    
    *message_out = msg_tmp;
    return 0;
}



