#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <string>
#include <sstream>
#include <functional>

#include <czmq.h>

#include "ymsg.h"
#include "bios_agent.h"
#include "agents.h"

#include "log.h"
#include "defs.h"
#include "str_defs.h"
#include "utils.h"
#include "utils_ymsg.h"
#include "utils_ymsg++.h"

#include "cm-agent-web.h"
#include "cm-agent-db.h"

#define DEFAULT_LOG_LEVEL LOG_INFO
#define STREAM_CONSUMER_PATTERN ""
#define STREAM_SUPPRESS 1

void usage () {
    printf ("Usage: %s [log_level]\n", BIOS_AGENT_NAME_COMPUTATION);
    printf ("Options\n");
    printf ("   log_level   One of the following values (default: LOG_INFO)\n");
    printf ("               LOG_DEBUG | LOG_INFO | LOG_WARNING | LOG_ERR | LOG_CRIT\n");
}

int main (int argc, char **argv) {

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

    // memento of pending requests (someone requests something from us)
    std::map <uint16_t, std::pair <std::string, ymsg_t *>> requests; 

    // TODO: Rewrite the std::function in rules to also return subject when returning message_out; edit fnctions accordingly
    // TODO: _later_ extend this with pattern once we start listening on stream
    std::map <std::string, std::function<int(bios_agent_t*,  ymsg_t *, std::map <uint16_t, std::pair <std::string, ymsg_t *>>&, const char*, ymsg_t **)>> rules;
    rules.emplace (std::make_pair ("return_measurements", process_db_measurement));
    rules.emplace (std::make_pair ("metric/computed/average", process_web_average));

    bios_agent_t *agent = bios_agent_new (MLM_ENDPOINT, BIOS_AGENT_NAME_COMPUTATION);
    if (agent == NULL) {
        log_critical ("bios_agent_new (\"%s\", \"%s\") failed.", bios_get_stream_main (), BIOS_AGENT_NAME_COMPUTATION);
        log_info ("%s finished.", BIOS_AGENT_NAME_COMPUTATION);
        return EXIT_FAILURE;
    }

#ifndef STREAM_SUPPRESS
    int rv = bios_agent_set_consumer (agent, bios_get_stream_main (), STREAM_CONSUMER_PATTERN);
    if (rv != 0) {
        log_error ("bios_agent_set_consumer (\"%s\", \".*\") failed.", bios_get_stream_main ());
        log_error ("%s WILL NOT be able to listen on %s.",BIOS_AGENT_NAME_COMPUTATION, bios_get_stream_main ());
    }
#endif

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

        // Note: I have valid objections to routing by subject and not the message itself, but 
        //       time is of essence and this can be rewritten anytime later

        auto needle = rules.find (subject);
        if (needle != rules.cend()) {
            ymsg_t *msg_out = NULL;
            rv = rules.at (subject) (agent, msg_recv, requests, sender, &msg_out);
            assert (rv != -1);
            if (msg_out != NULL) {
                ymsg_format (msg_out, msg_print);                
                rv = bios_agent_replyto (agent, sender, "", &msg_out, msg_recv); // msg_out is destroyed
                if (rv != 0) {
                    log_critical ("bios_agent_replyto (\"%s\", \"%s\") failed.", sender, "");
                }
                else {
                    log_debug ("ACTION: message sent to %s with subject %s:\n%s", sender, "", msg_print.c_str ());
                }
            }
        }
        else {
            log_warning ("ACTION. Don't know what to do with this message.");
        }
        ymsg_destroy (&msg_recv); // msg_recv is destroyed
    }        

    bios_agent_destroy (&agent);
    log_info ("%s finished.", BIOS_AGENT_NAME_COMPUTATION);
    log_close ();
    return EXIT_SUCCESS;
}

