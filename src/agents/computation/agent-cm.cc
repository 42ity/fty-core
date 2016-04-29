/*
 *
 * Copyright (C) 2015 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*!
 * \file agent-cm.cc
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \author Jim Klimov <EvgenyKlimov@Eaton.com>
 * \brief Not yet documented file
 */
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <string>
#include <sstream>
#include <functional>
#include <map>

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

#include "preproc.h"

namespace cm = computation;

int main (UNUSED_PARAM int argc, UNUSED_PARAM char **argv) {

    log_open ();
    log_info ("%s started.", BIOS_AGENT_NAME_COMPUTATION);

    std::map <std::string, std::function<void (bios_agent_t*, ymsg_t *, const char*, ymsg_t **)>> rules;
    rules.emplace (std::make_pair ("metric/computed/average", cm::web::process));

    _scoped_bios_agent_t *agent = bios_agent_new (MLM_ENDPOINT, BIOS_AGENT_NAME_COMPUTATION);
    if (agent == NULL) {
        log_critical ("bios_agent_new (endpoint = \"%s\", name = \"%s\") failed.", MLM_ENDPOINT, BIOS_AGENT_NAME_COMPUTATION);
        return EXIT_FAILURE;
    }

    int rv = bios_agent_set_producer (agent, "METRICS");
    if (rv != 0) {
        log_critical ("bios_agent_set_producer (stream = '%s') failed.", "METRICS");
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
    
    // We don't really need a poller. We just have one client (actor/socket)
    while (!zsys_interrupted) {
        log_debug ("WAITING");
        _scoped_ymsg_t *msg_recv = bios_agent_recv (agent);
        const char *subject = safe_str (bios_agent_subject (agent)); 
        const char *pattern = safe_str (bios_agent_command (agent));
        const char *sender = safe_str (bios_agent_sender (agent));
        const char *recipient = safe_str (bios_agent_address (agent));

        std::string msg_print; // formatted message

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
            needle->second (agent, msg_recv, sender, &msg_out);
            if (msg_out != NULL) {
                ymsg_format (msg_out, msg_print);                
                rv = bios_agent_replyto (agent, sender, "", &msg_out, msg_recv); // msg_out is destroyed
                if (rv != 0) {
                    log_error ("bios_agent_replyto (address = \"%s\", subject = \"%s\") failed.", sender, "");
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
    return EXIT_SUCCESS;
}
