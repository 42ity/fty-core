#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <string>
#include <sstream>
#include <functional>
#include <tntdb/connect.h>

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

#include "cm-web.h"
#include "cleanup.h"
#include "dbpath.h"

#include "preproc.h"

#define DEFAULT_LOG_LEVEL LOG_INFO

namespace cm = computation;

int main (UNUSED_PARAM int argc, UNUSED_PARAM char **argv) {

    int log_level = DEFAULT_LOG_LEVEL;
    
    char *ev_log_level = getenv (EV_BIOS_LOG_LEVEL);

    if (ev_log_level) {
        if (strcmp (ev_log_level, STR (LOG_DEBUG)) == 0) {
            log_level = LOG_DEBUG;
        }
        else if (strcmp (ev_log_level, STR (LOG_INFO)) == 0) {
            log_level = LOG_INFO;
        }
        else if (strcmp (ev_log_level, STR (LOG_WARNING)) == 0) {
            log_level = LOG_WARNING;
        }
        else if (strcmp (ev_log_level, STR (LOG_ERR)) == 0) {
            log_level = LOG_ERR;
        }
        else if (strcmp (ev_log_level, STR (LOG_CRIT)) == 0) {
            log_level = LOG_CRIT;
        }
    }
    log_open ();
    log_set_level (log_level);
    log_info ("%s started.", BIOS_AGENT_NAME_COMPUTATION);

    std::map <std::string, std::function<void(tntdb::Connection& conn, bios_agent_t*, ymsg_t *, const char*, ymsg_t **)>> rules;
    rules.emplace (std::make_pair ("metric/computed/average", cm::web::process));

    _scoped_bios_agent_t *agent = bios_agent_new (MLM_ENDPOINT, BIOS_AGENT_NAME_COMPUTATION);
    if (agent == NULL) {
        log_critical ("bios_agent_new (endpoint = \"%s\", name = \"%s\") failed.", bios_get_stream_main (), BIOS_AGENT_NAME_COMPUTATION);
        return EXIT_FAILURE;
    }

    int rv = bios_agent_set_producer (agent, bios_get_stream_main ());    
    if (rv < 0) {
        log_critical ("bios_agent_set_producer (stream = '%s') failed.", bios_get_stream_main ());
        bios_agent_destroy (&agent);       
        return EXIT_FAILURE;
    }

#define STREAM_SUPPRESS 1
#ifndef STREAM_SUPPRESS
    // Note: If we ever start listening on a stream here, then:
    //  - There will be a separate stream for computation requests (as is planned for future, to replace the one stream)
    //  - Map of 'subject' -> std::function will NOT be used to process these messages.
    //    Instead using bios_agent_command () we'll recognize a message came from "STREAM DELIVER" pattern
    //    and a different function to process all of stream requests will be called.
    rv = bios_agent_set_consumer (agent, bios_get_stream_main (), "");
    if (rv != 0) {
        log_error ("bios_agent_set_consumer (\"%s\", \".*\") failed.", bios_get_stream_main ());
        log_error ("%s WILL NOT be able to listen on %s.",BIOS_AGENT_NAME_COMPUTATION, bios_get_stream_main ());
    }
#endif
    
    tntdb::Connection conn;
    try {
        conn = tntdb::connectCached (url);
    }
    catch (...) { // TODO: std::exception&...
        log_critical ("tntdb::connnectCached faile.");
        return EXIT_FAILURE;
    } 
    // We don't really need a poller. We just have one client (actor/socket)
    while (!zsys_interrupted) {
        log_debug ("WAITING");
        _scoped_ymsg_t *msg_recv = bios_agent_recv (agent);
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
            _scoped_ymsg_t *msg_out = NULL;
            needle->second (conn, agent, msg_recv, sender, &msg_out);
            if (msg_out != NULL) {
                ymsg_format (msg_out, msg_print);                
                rv = bios_agent_replyto (agent, sender, "", &msg_out, msg_recv); // msg_out is destroyed
                if (rv != 0) {
                    log_critical ("bios_agent_replyto (address = \"%s\", subject = \"%s\") failed.", sender, "");
                }
                else {
                    log_debug ("ACTION: message sent to '%s' with subject '%s':\n%s", sender, "", msg_print.c_str ());
                }
            }
        }
        else {
            log_warning ("ACTION. Don't know what to do with this message => Throwing away.");
        }
        ymsg_destroy (&msg_recv); // msg_recv is destroyed
    }        

    bios_agent_destroy (&agent);
    log_info ("%s finished.", BIOS_AGENT_NAME_COMPUTATION);
    log_close ();
    return EXIT_SUCCESS;
}

