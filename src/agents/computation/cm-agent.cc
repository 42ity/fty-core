#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <string>
#include <sstream>
#include <cmath>

#include <czmq.h>
#include <cxxtools/jsondeserializer.h>
#include <cxxtools/serializationerror.h>

#include "ymsg.h"
#include "bios_agent.h"
#include "agents.h"

#include "log.h"
#include "defs.h"
#include "str_defs.h"
#include "utils.h"
#include "utils_ymsg.h"
#include "utils_ymsg++.h"


#define DEFAULT_LOG_LEVEL LOG_INFO
#define STREAM_CONSUMER_PATTERN ".*"

void usage () {
    printf ("Usage: %s [log_level]\n", BIOS_AGENT_NAME_COMPUTATION);
    printf ("Options\n");
    printf ("   log_level   One of the following values (default: LOG_INFO)\n");
    printf ("               LOG_DEBUG | LOG_INFO | LOG_WARNING | LOG_ERR | LOG_CRIT\n");
}

/* Note to self:
 * So far the cases are manageable, but we might want to try out either wrapping czmq primitives with classes+deleters and with smart pointers 
 * or some other techniques or this will soon get out of hand.
 */



// TODO: ugly, rewrite
int calculate (std::map <int64_t, double>& samples, int64_t start, int64_t end, const char *type, double& result) {
    if (!type || (start >= end) ) {
        return -1;
    }
    auto it_start = samples.find (start);
    auto it_end = samples.find (end);
    if (it_start == samples.cend () || it_end == samples.cend())
        return -1;

    if (strcmp (type, "min") == 0) {
        double minimum = it_start->second;
        for (auto s = it_start; s != it_end; ++s) {
            if (s->second < minimum)
                minimum = s->second;
        }
        result = minimum;
        return 0;
    }
    else if (strcmp (type, "max") == 0) {
        double maximum = it_start->second;
        for (auto s = it_start; s != it_end; ++s) {
            if (s->second > maximum)
                maximum = s->second;
        }
        result = maximum;
        return 0;
    }
    else if (strcmp (type, "arithmetic_mean") == 0) {
        double arithmetic_mean = 0;
        uint64_t counter = 0;
        // prevent overflow
        for (auto s = it_start; s != it_end; ++s) {
            ++counter;
        }
        for (auto s = it_start; s != it_end; ++s) {
            arithmetic_mean += (s->second / counter);
        }
        result = arithmetic_mean;
        return 0;
    }
    return -1;
}


int main (int argc, char **argv) {
    zsys_catch_interrupts ();

    int log_level = DEFAULT_LOG_LEVEL;
    if (argc > 1) {
        if (strcmp (argv[1], STR (LOG_DEBUG)) == 0) {
            log_level = LOG_DEBUG;
        }
        else if (strcmp (argv[1], STR (LOG_INFO)) == 0) {
            log_level = LOG_INFO;
        }
        else if (strcmp (argv[1], STR (LOG_WARNING)) == 0) {
            log_level = LOG_WARNING;
        }
        else if (strcmp (argv[1], STR (LOG_ERR)) == 0) {
            log_level = LOG_ERR;
        }
        else if (strcmp (argv[1], STR (LOG_CRIT)) == 0) {
            log_level = LOG_CRIT;
        }
        else {
            usage ();
            return EXIT_FAILURE;
        }
    }
    log_open ();
    log_set_level (log_level);
    log_info ("%s started.", BIOS_AGENT_NAME_COMPUTATION);

    // structure of request (someone requests something from us)
    std::map <uint16_t, std::pair <std::string, ymsg_t *>> requests; 

    bios_agent_t *agent = bios_agent_new (MLM_ENDPOINT, BIOS_AGENT_NAME_COMPUTATION);
    if (agent == NULL) {
        log_critical ("bios_agent_new (\"%s\", \"%s\") failed.", bios_get_stream_main (), BIOS_AGENT_NAME_COMPUTATION);
        log_info ("%s finished.", BIOS_AGENT_NAME_COMPUTATION);
        return EXIT_FAILURE;
    }

    int rv = bios_agent_set_consumer (agent, bios_get_stream_main (), STREAM_CONSUMER_PATTERN);
    if (rv != 0) {
        log_error ("bios_agent_set_consumer (\"%s\", \".*\") failed.", bios_get_stream_main ());
        log_error ("%s WILL NOT be able to listen on %s.",BIOS_AGENT_NAME_COMPUTATION, bios_get_stream_main ());
    }

    // We don't really need a poller. We just have one client (actor/socket)
    while (!zsys_interrupted) {
        ymsg_t *msg_recv = bios_agent_recv (agent);
        const char *subject = safe_str (bios_agent_subject (agent)); 
        const char *pattern = safe_str (bios_agent_command (agent));
        const char *sender = safe_str (bios_agent_sender (agent));
        const char *recipient = safe_str (bios_agent_address (agent));

        std::string msg_print; // formatted message
        int rv = 0; // return value of functions

        ymsg_format (msg_recv, msg_print);
        log_debug ("RECEIVED\n\tSender: '%s', address: '%s', pattern: '%s',  subject: '%s', status: '%d'.\n\tMessage:\n%s", 
            sender, recipient, pattern, subject, bios_agent_status (agent), msg_print.c_str());

        if (!msg_recv) {
            log_debug ("ACTION. N/A");            
            continue;
        }

        zrex_t *rest_regex = zrex_new ("^rest\\.\\d+$");
        assert (rest_regex);

        if (zrex_matches (rest_regex, sender)) { // TNTNET
            log_debug ("BRANCH: web");
            if (strcmp (subject, "metric/computed/average") == 0) {     // metric/computed/average
                log_debug ("BRANCH: metric/computed/average");
                // average request decode variables
                int64_t start_ts = -1, end_ts = -1;
                char *type = NULL, *step = NULL, *source = NULL;
                uint64_t element_id = 0;
                rv = bios_web_average_request_decode (msg_recv, &start_ts, &end_ts, &type, &step, &element_id, &source);
                if (rv != 0) {
                    // log and send reply to TNTNET 
                    log_error ("bios_web_average_request_decode () failed.");                    
                    goto tntnet_reply_msg_err;
                }
                // Prepare measurements read request
                char *send_subject = NULL;
                // When user says, e.g.: "I want 8h averages starting from 23:15:.... " we need to request sampled data from an earlier period
                // in order to compute the first 8h average. This first average belongs to 24:00:00 and is computed from <16:00, 24:00).
                // Additional AGENT_NUT_REPEAT_INTERVAL_SEC seconds are deducted to complete possible leading values `missing`.
                start_ts = computable_interval (start_ts, step); // shared/utils.c 
                // we are requesting sampled data from db, therefore no topic construction (i.e. <source>.<type>_<step>) is needed.
                ymsg_t *msg_send = bios_db_measurements_read_request_encode (start_ts, end_ts, element_id, source, &send_subject);
                if (type) {
                    free (type);
                    type = NULL;
                }
                if (step) {
                    free (step);
                    step = NULL;
                }
                if (source) {
                    free (source);
                    source = NULL;
                }
                assert (msg_send);
                // 'seq' of the next outcoming message
                uint16_t seq = bios_agent_seq (agent);
                ymsg_format (msg_send, msg_print);
                // send the request to db-ng
                rv = bios_agent_sendto (agent, BIOS_AGENT_NAME_DB_MEASUREMENT, send_subject, &msg_send); // msg_send is destroyed
                free (send_subject); send_subject = NULL;
                if (rv != 0) {
                    log_critical ("bios_agent_sendto (\"%s\", \"%s\") failed.", BIOS_AGENT_NAME_DB_MEASUREMENT, send_subject);
                    goto tntnet_reply_msg_err;
                }
                requests.emplace (std::make_pair (seq, std::make_pair (std::string(sender), msg_recv))); // msg_recv is saved
                log_debug ("ACTION: Message sent to %s\n%s", BIOS_AGENT_NAME_DB_MEASUREMENT, msg_print.c_str ());
            } // metric/computed/average end
            else {
                ymsg_format (msg_recv, msg_print);
                ymsg_destroy (&msg_recv); // msg_recv is destroyed
                log_warning ("ACTION. Don't know what to do with this message.");
                continue;
            }
            continue;
tntnet_reply_msg_err:
            ymsg_t *msg_reply = ymsg_new (YMSG_REPLY); 
            assert (msg_reply);
            ymsg_set_status (msg_reply, false);
            ymsg_set_errmsg (msg_reply, "Internal error. Please check logs.");
            ymsg_format (msg_reply, msg_print);
            rv = bios_agent_replyto (agent, sender, "", &msg_reply, msg_recv); // msg_reply is destroyed
            ymsg_destroy (&msg_recv); // msg_recv is destroyed
            if (rv != 0) {
                log_critical ("bios_agent_replyto (\"%s\", \"%s\") failed.", sender, "");
            }
            else {
                log_debug ("ACTION. Error message sent to %s\n%s", sender, msg_print.c_str ());
            }
            continue;
        } // TNTNET end
        else if (strcmp (sender, BIOS_AGENT_NAME_DB_MEASUREMENT) == 0) { // db measurement
            log_debug ("BRANCH: %s", BIOS_AGENT_NAME_DB_MEASUREMENT);
            if (ymsg_id (msg_recv) == YMSG_SEND) {
                ymsg_format (msg_recv, msg_print);
                ymsg_destroy (&msg_recv); // msg_recv is destroyed
                log_error ("%s expects only YMSG_REPLY messages from %s.", BIOS_AGENT_NAME_COMPUTATION, BIOS_AGENT_NAME_DB_MEASUREMENT);
                // YMSG_SEND doesn't have 'rep' field so we can't search our map
                continue;
            }
            // get 'rep' (which is the 'seq' of the original request == key to our map)
            uint16_t rep = ymsg_rep (msg_recv);
            std::map <uint16_t, std::pair <std::string, ymsg_t *>>::const_iterator requests_it = requests.find (rep);
            std::string to_address;
            ymsg_t *msg_orig = NULL;

            if (requests_it != requests.cend ()) {
                to_address.assign (requests_it->second.first);
                msg_orig = requests_it->second.second;
                requests.erase (requests_it);
            }
            else {
                ymsg_format (msg_recv, msg_print);
                ymsg_destroy (&msg_recv); // msg_recv is destroyed
                log_error ("Can't find 'rep' value in internal structure of sent out 'seq' numbers.");
                // We don't have a record in map, so we don't know whom to send message
                continue;
            }
            // sanity check
            assert ( msg_orig != NULL );
            

            // first let's check status of the received message
            if (!ymsg_is_ok (msg_recv)) {
                log_debug ("Message status = error.");
                ymsg_t *msg_reply = ymsg_new (YMSG_REPLY);
                assert (msg_reply);
                ymsg_set_status (msg_reply, false);
                if (ymsg_errmsg (msg_recv)) {
                    ymsg_set_errmsg (msg_reply, ymsg_errmsg (msg_recv));
                }
                ymsg_format (msg_reply, msg_print);
                rv = bios_agent_replyto (agent, to_address.c_str (), "", &msg_reply, msg_orig); // msg_reply is destroyed
                if (rv != 0) {
                    log_critical ("bios_agent_replyto (\"%s\", \"%s\") failed.", to_address.c_str(), "");
                }
                else {                
                    log_debug ("ACTION: Error message sent to %s\n%s", to_address.c_str (), msg_print.c_str ());
                }
                ymsg_destroy (&msg_recv); // msg_recv is destroyed
                ymsg_destroy (&msg_orig); // msg_orig is destroyed
                continue;
            }
            
            char *json_in = NULL;
            rv = bios_db_measurements_read_reply_decode (msg_recv, &json_in);
            ymsg_destroy (&msg_recv);
            if (rv != 0 || !json_in || strlen (json_in) == 0) {
                // log and send reply to to_address
                log_error ("bios_db_measurements_read_reply_decode () failed or json empty or NULL.");
                ymsg_t *msg_reply = ymsg_new (YMSG_REPLY);
                assert (msg_reply);
                ymsg_set_status (msg_reply, false);
                ymsg_set_errmsg (msg_reply, "Internal error."); // TODO: better error message
                ymsg_format (msg_reply, msg_print);
                rv = bios_agent_replyto (agent, to_address.c_str (), "", &msg_reply, msg_orig); // msg_reply is destroyed
                if (rv != 0) {
                    log_critical ("bios_agent_replyto (\"%s\", \"%s\") failed.", to_address.c_str (), "");
                }
                else {
                    log_debug ("ACTION: Error message sent to %s\n%s", to_address.c_str (), msg_print.c_str ());
                }
                ymsg_destroy (&msg_orig);
                continue;
            }
            log_debug ("bios_db_measurements_read_reply_decode successfull.");
            printf ("json:\n%s\n", json_in); // TODO: remove when done testing

            // process the received json into a std::map
            std::stringstream input (json_in, std::ios_base::in);
            if (json_in) {
                free (json_in);
                json_in = NULL;
            }
            cxxtools::SerializationInfo si;
            cxxtools::JsonDeserializer deserializer (input);

            std::string source, unit;
            int64_t start_ts = -1, end_ts = -1;
            uint64_t element_id;
            std::map <int64_t, double> samples;

            ymsg_t *msg_reply = NULL;
            try {
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
                start_ts = std::stoll (start_ts_str);
                end_ts = std::stoll (end_ts_str);

                log_debug("start_ts: %ld\tend_ts: %ld\tsource: %s\tunit: %s\telement_id: %ld", start_ts, end_ts, source.c_str (), unit.c_str (), element_id);

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
                    printf ("value: %s\tscale: %s\ttimestamp: %s\tcomputed_value: %f", value.c_str(), scale.c_str(), timestamp.c_str(), comp_value);
                    samples.emplace (std::make_pair ( std::stoll (timestamp), comp_value));

                }

                log_debug ("samples map size before post-calculation: %ld\n", samples.size ());                            

                // decode the stored (original) request
                int64_t orig_start_ts, orig_end_ts;
                uint64_t orig_element_id;
                char *orig_type = NULL, *orig_step = NULL, *orig_source = NULL;

                assert (msg_orig);
                rv = bios_web_average_request_decode (msg_orig, &orig_start_ts, &orig_end_ts, &orig_type, &orig_step, &orig_element_id, &orig_source);
                if (rv != 0) { // this should never happen; the stored message had already been successfully decoded once
                    // log error and send to to_address
                    log_critical ("bios_web_average_request_decode () failed.");
                    assert (msg_reply == NULL);
                    ymsg_t *msg_reply = ymsg_new (YMSG_REPLY); 
                    assert (msg_reply);
                    ymsg_set_status (msg_reply, false);
                    ymsg_set_errmsg (msg_reply, "Internal error.");
                    ymsg_format (msg_reply, msg_print);
                    rv = bios_agent_replyto (agent, to_address.c_str (), "", &msg_reply, msg_orig); // msg_reply is destroyed
                    if (rv != 0) {
                        log_critical ("bios_agent_replyto (\"%s\", \"%s\") failed.", to_address.c_str (), "");
                    }
                    else {
                        log_debug ("ACTION. Error message sent to %s\n%s", sender, msg_print.c_str ());
                    }                 
                    ymsg_destroy (&msg_orig);
                    continue;
                }
                // sanity check
                // this also should never happen; db-ng should not alter the requested values
                if ((element_id != orig_element_id) ||
                    (end_ts != orig_end_ts) ||
                    (source.compare (orig_source) != 0)) {
                    // log and send reply to to_address
                    log_error ("requested and returned values don't match"); //TODO
                    assert (msg_reply == NULL);
                    ymsg_t *msg_reply = ymsg_new (YMSG_REPLY); 
                    assert (msg_reply);
                    ymsg_set_status (msg_reply, false);
                    ymsg_set_errmsg (msg_reply, "Internal error.");
                    ymsg_format (msg_reply, msg_print);
                    rv = bios_agent_replyto (agent, to_address.c_str (), "", &msg_reply, msg_orig); // msg_reply is destroyed
                    if (rv != 0) {
                        log_critical ("bios_agent_replyto (\"%s\", \"%s\") failed.", to_address.c_str (), "");
                    }
                    else {
                        log_debug ("ACTION. Error message sent to %s\n%s", sender, msg_print.c_str ());
                    }                 
                    ymsg_destroy (&msg_orig);
                    continue;
                }


                int64_t samples_first_ts = samples.cbegin()->first,
                        samples_last_ts = (--samples.cend())->first;
                log_debug ("Timestamp of first sample = %ld\tTimestamp of last sample = %ld", samples_first_ts, samples_last_ts);

                // align with sampling interval
                int64_t postcalc_ts = samples_first_ts + (AGENT_NUT_SAMPLING_INTERVAL_SEC - (samples_first_ts % AGENT_NUT_SAMPLING_INTERVAL_SEC));
                // Calculate/fill in the gaps
                while (postcalc_ts  < end_ts) {
                    auto needle = samples.find (postcalc_ts);
                    if (needle != samples.end ()) {
                        postcalc_ts += AGENT_NUT_SAMPLING_INTERVAL_SEC;
                        continue;
                    }
                    std::map<int64_t, double>::iterator it, it2;
                    bool inserted;
                    std::tie (it, inserted) = samples.emplace (std::make_pair (postcalc_ts, (double) 0));
                    assert (inserted == true);
                    it2 = it; --it2;
                    it->second = (it2)->second;
                }

                   
                // TODO: remove when done testing
                for (const auto &p : samples) {
                    std::cout << p.first << " => " << p.second << '\n';
                }
                printf ("Now the intervals:\n");
                int64_t step_sec = average_step_seconds (orig_step);
                assert (step_sec != -1);           
                int64_t cut_start = samples_first_ts;
                int64_t cut_end = samples_first_ts - (samples_first_ts % step_sec) + step_sec;

                std::string data_str;
                int comma_counter = 0;
                do {
                    std::string item = BIOS_WEB_AVERAGE_REPLY_JSON_DATA_ITEM_TMPL;
                    double comp_result;
                    rv = calculate (samples, cut_start, cut_end, orig_type, comp_result);
                    assert (rv == 0);

                    printf ("%ld\t%ld\t%f\n", cut_start, cut_end, comp_result);
                    item.replace (item.find ("##VALUE##"), strlen ("##VALUE##"), std::to_string (comp_result));
                    item.replace (item.find ("##TIMESTAMP##"), strlen ("##TIMESTAMP##"), std::to_string (cut_end));
                    cut_start = cut_end;
                    cut_end += step_sec;
                    if (comma_counter == 0) 
                        ++comma_counter;
                    else
                        data_str += ",\n";

                    data_str += item;

                } while (cut_end < end_ts);
                    
                // TODO
                std::string json_out(BIOS_WEB_AVERAGE_REPLY_JSON_TMPL);           
                json_out.replace (json_out.find ("##UNITS##"), strlen ("##UNITS##"), unit);
                json_out.replace (json_out.find ("##SOURCE##"), strlen ("##SOURCE##"), source);
                json_out.replace (json_out.find ("##STEP##"), strlen ("##STEP##"), orig_step);
                json_out.replace (json_out.find ("##TYPE##"), strlen ("##TYPE##"), orig_type);
                json_out.replace (json_out.find ("##ELEMENT_ID##"), strlen ("##ELEMENT_ID##"), std::to_string (element_id));
                json_out.replace (json_out.find ("##START_TS##"), strlen ("##START_TS##"), std::to_string (orig_start_ts));
                json_out.replace (json_out.find ("##END_TS##"), strlen ("##END_TS##"), std::to_string (orig_end_ts));
                if (orig_type) {
                    free (orig_type);
                    orig_type = NULL;
                }
                if (orig_step) {
                    free (orig_step);
                    orig_step = NULL;
                }
                if (orig_source) {
                    free (orig_source);
                    orig_source = NULL;
                }
                               
                json_out.replace (json_out.find ("##DATA##"), strlen ("##DATA##"), data_str);
                
                assert (msg_reply == NULL);
                ymsg_t *msg_reply = ymsg_new (YMSG_REPLY);
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
                continue;
            }
            catch (const std::exception& e) {
                ymsg_destroy (&msg_orig);
                ymsg_destroy (&msg_reply);
                log_error ("std::exception caught: %s", e.what ());
                continue;
            }
            catch (...) {
                ymsg_destroy (&msg_orig);
                ymsg_destroy (&msg_reply);
                log_error ("unknown exception caught");
            }

       } // db measurement end
       else {                       
            ymsg_format (msg_recv, msg_print);
            ymsg_destroy (&msg_recv); // msg_recv is destroyed
            log_warning ("ACTION. Don't know what to do with this message.");
            continue;
       }
    }        


/* poller.
    zpoller_t *poller = NULL;
    // TODO poller_new ( ,NULL);
    // assert (poller);
    
    while (!zpoller_terminated (poller)) {
        // neco which = 
        zpoller_wait (poller, -1);
    }
    
    zpoller_destroy (&poller);
*/
    bios_agent_destroy (&agent);
    log_info ("%s finished.", BIOS_AGENT_NAME_COMPUTATION);
    log_close ();
    return EXIT_SUCCESS;
}
