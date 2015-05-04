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
 \file   cm-agent-db.cc
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
#include "cm-agent-utils.h"

#include "cm-agent-db.h"
#include "cleanup.h"



int
process_db_measurement
(bios_agent_t *agent, ymsg_t *message_in, std::map <uint16_t, std::pair <std::string, ymsg_t *>>& requests, const char *sender, ymsg_t **message_out) {
    if (!agent || !sender || !message_in || !message_out)
        return -1;
    
    std::string msg_print;
    if (ymsg_id (message_in) == YMSG_SEND) {
        ymsg_format (message_in, msg_print);
        log_error ("YMSG_REPLY expected");
        // YMSG_SEND doesn't have 'rep' field so we can't search our map
        return 0;
    }
    // get 'rep' (which is the 'seq' of the original request == key to our map)
    uint16_t rep = ymsg_rep (message_in);
    std::map <uint16_t, std::pair <std::string, ymsg_t *>>::const_iterator requests_it = requests.find (rep);
    std::string to_address;
    _scoped_ymsg_t *msg_orig = NULL;
    int rv = 0;

    if (requests_it != requests.cend ()) {
        to_address.assign (requests_it->second.first);
        msg_orig = requests_it->second.second;
        requests.erase (requests_it);
    }
    else {
        ymsg_format (message_in, msg_print);
        log_error ("Can't find 'rep' value in internal structure of sent out 'seq' numbers.");
        // We don't have a record in map, so we don't know whom to send message
        return 0;
    }
    // sanity check
    assert ( msg_orig != NULL );

    // first let's check status of the received message
    if (!ymsg_is_ok (message_in)) {
        log_debug ("Message status = error.");
        rv = replyto_err (agent, &msg_orig, to_address.c_str (), "Internal error. Check logs.", ""); // msg_orig is destroyed;
                                                                                                // TODO: when subject added to output args...
        assert (rv == 0);                                                                                               
        return 0;
    }
    
    char *json_in = NULL;
    rv = bios_db_measurements_read_reply_extract (message_in, &json_in);
    if (rv != 0 || !json_in || strlen (json_in) == 0) {
        log_error ("bios_db_measurements_read_reply_extract () failed or json empty or NULL.");
        rv = replyto_err (agent, &msg_orig, to_address.c_str (), "Internal error. Check logs.", ""); // msg_orig is destroyed;
                                                                                                // TODO: when subject added to output args...
        assert (rv == 0);
        return 0;       
    }
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
        ymsg_destroy (&msg_orig);
        // TODO: return and ?
    }

    _scoped_ymsg_t *msg_reply = NULL;
    try {

        log_debug ("samples map size before post-calculation: %ld\n", samples.size ());                            

        // decode the stored (original) request
        int64_t orig_start_ts, orig_end_ts;
        uint64_t orig_element_id;
        char *orig_type = NULL, *orig_step = NULL, *orig_source = NULL;

        assert (msg_orig);
        rv = bios_web_average_request_extract (msg_orig, &orig_start_ts, &orig_end_ts, &orig_type, &orig_step, &orig_element_id, &orig_source);
        if (rv != 0) { // this should never happen; the stored message had already been successfully decoded once
            // log error and send to to_address
            log_critical ("bios_web_average_request_extract () failed.");
            rv = replyto_err (agent, &msg_orig, to_address.c_str (), "Internal error. Check logs.", ""); // msg_orig is destroyed;
                                                                                                    // TODO: when subject added to output args...
            assert (rv == 0);
            return 0;
        }
        // sanity check
        // this also should never happen; db-ng should not alter the requested values
        if ((element_id != orig_element_id) ||
            (end_ts != orig_end_ts) ||
            (source.compare (orig_source) != 0)) {
            // log and send reply to to_address
            log_error ("requested and returned values don't match"); //TODO
            rv = replyto_err (agent, &msg_orig, to_address.c_str (), "Internal error. Check logs.", ""); // msg_orig is destroyed;
                                                                                                    // TODO: when subject added to output args...
            assert (rv == 0);

            return 0;
        }

        process_db_measurement_solve_left_margin (samples, start_ts);
        // TODO: remove when done testing
        for (const auto &p : samples) {
            std::cout << p.first << " => " << p.second << '\n';
        }

        int64_t first_ts = samples.cbegin()->first;
        int64_t second_ts = average_first_since (first_ts, orig_step);
        double comp_result;

        std::string data_str;
        int comma_counter = 0;

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
        }

        // TODO
        std::string json_out(BIOS_WEB_AVERAGE_REPLY_JSON_TMPL);           
        json_out.replace (json_out.find ("##UNITS##"), strlen ("##UNITS##"), unit);
        json_out.replace (json_out.find ("##SOURCE##"), strlen ("##SOURCE##"), source);
        json_out.replace (json_out.find ("##STEP##"), strlen ("##STEP##"), orig_step);
        json_out.replace (json_out.find ("##TYPE##"), strlen ("##TYPE##"), orig_type);
        json_out.replace (json_out.find ("##ELEMENT_ID##"), strlen ("##ELEMENT_ID##"), std::to_string (element_id));
        json_out.replace (json_out.find ("##START_TS##"), strlen ("##START_TS##"), std::to_string (orig_start_ts));
        json_out.replace (json_out.find ("##END_TS##"), strlen ("##END_TS##"), std::to_string (orig_end_ts));
        FREE0 (orig_type)
        FREE0 (orig_step)
        FREE0 (orig_source)
                      
        json_out.replace (json_out.find ("##DATA##"), strlen ("##DATA##"), data_str);
        
        assert (msg_reply == NULL);
        _scoped_ymsg_t *msg_reply = ymsg_new (YMSG_REPLY);
        assert (msg_reply);
        ymsg_set_status (msg_reply, true);
        zchunk_t *chunk = zchunk_new (json_out.c_str (), json_out.size ());
        ymsg_set_response (msg_reply, &chunk);
        // return message to rest
        ymsg_format (msg_reply, msg_print);
        rv = bios_agent_replyto (agent, to_address.c_str (), "", &msg_reply, msg_orig); // msg_reply is destroyed
        if (rv != 0) {
            log_critical ("bios_agent_replyto (\"%s\", \"%s\") failed.", to_address.c_str (), "");
        }
        else {
            log_debug ("ACTION: Message sent to %s\n%s", to_address.c_str (), msg_print.c_str ());
        }
        ymsg_destroy (&msg_orig);

    }
    // it's safe to call msg_destroy on uninitialized ymsg_t*
    catch (const cxxtools::SerializationError& e) {
        ymsg_destroy (&msg_orig);
        ymsg_destroy (&msg_reply);
        log_error ("cxxtools::SerializationError caught: %s", e.what ());
        
    }
    catch (const std::exception& e) {
        ymsg_destroy (&msg_orig);
        ymsg_destroy (&msg_reply);
        log_error ("std::exception caught: %s", e.what ());
    }
    catch (...) {
        ymsg_destroy (&msg_orig);
        ymsg_destroy (&msg_reply);
        log_error ("unknown exception caught");
    }
    return 0;
}

