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
 * \file agent-dbstore.cc
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 * \author Michal Vyskocil <MichalVyskocil@Eaton.com>
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \author Tomas Halman <TomasHalman@Eaton.com>
 * \brief Not yet documented file
 */
#include "cleanup.h"
#include "malamute.h"
#include "monitor.h"
#include "ymsg.h"
#include "bios_agent.h"
#include "persistencelogic.h"
#include "src/persist/measurement.h"
#include "dbpath.h"
#include "log.h"
#include "defs.h"
#include "str_defs.h"
#include "ymsg-asset.h"

#include <stdio.h>
#include <zsys.h>

//probably not needed
static const char*
s_safe_str(const char* s) {
    return s ? s : "<null>";
}

#define _s s_safe_str

int main (int argc, char *argv []) {
    
    log_open();
    log_info ("## agent: %s started", BIOS_AGENT_NAME_DB_MEASUREMENT);
    
    // Basic settings
    if (argc > 3) {
        printf ("syntax: agent-dbstore [ <endpoint> | <endpoint> <mysql:db=bios;user=bios;password=test> ]\n");
        return 1;
    }
    const char *addr = (argc == 1) ? "ipc://@/malamute" : argv[1];
    if (argc > 2) {
        url = argv[2];
    }

    // Create a client
    bios_agent_t *client = bios_agent_new(addr, BIOS_AGENT_NAME_DB_MEASUREMENT);
    if(!client) {
        log_error ("agent-dbstore: error bios_agent_new ()");
        return 1;
    }

    persist::TopicCache c{256};

    // We are listening for measurements
    bios_agent_set_consumer(client, bios_get_stream_main(), ".*"); // to be dropped onc we migrate to multiple streams
    bios_agent_set_consumer(client, bios_get_stream_measurements(), ".*");
    while(!zsys_interrupted) {

        _scoped_ymsg_t *in = bios_agent_recv(client);
        if ( in == NULL )
            continue;
        log_debug ("command: '%s', reason: '%s', address: '%s', sender: '%s', subject: '%s'",\
                _s(bios_agent_command(client)),
                _s(bios_agent_reason(client)),
                _s(bios_agent_address(client)),
                _s(bios_agent_sender(client)),
                _s(bios_agent_subject(client))
                );

        const char *command = bios_agent_command(client);

        // Mailbox deliver is direct message
        if (streq (command, "MAILBOX DELIVER")) {
            if (ymsg_id(in) != YMSG_SEND) {
                log_warning("Got REPLY ymsg, that looks weird, discarding!\n");
                ymsg_destroy(&in);
                continue;
            }
            _scoped_ymsg_t *out = NULL;
            char* out_subj = NULL;
            persist::process_mailbox_deliver(&out, &out_subj, in, bios_agent_subject(client));
            if ( ( out_subj == NULL ) || ( out == NULL ) ){
                ymsg_destroy(&in);
                ymsg_destroy(&out);
                continue;
            }
            bios_agent_replyto(client, (char*)bios_agent_sender(client), out_subj, &out, in );
            ymsg_destroy(&out);
            free(out_subj);
        }
        // stream deliver
        else if (streq (command, "STREAM DELIVER")) {

            const char *stream = bios_agent_address(client);
            if (strncmp(bios_agent_subject(client), "inventory", 9) == 0 ) {
                log_debug ("inventory message recieved, ingore it. In future this should never happen");
            }
            else if (strncmp(bios_agent_subject(client), "configure", 9) == 0 ) {
                log_debug ("configure message recieved, ingore it. In future this should never happen");
            }
            else if (strncmp(bios_agent_subject(client), "alert.", 6) == 0 ) {
                ymsg_t* out = NULL;
                char* out_subj = NULL;
                persist::process_alert(&out, &out_subj, in,bios_agent_subject(client));
            }
            else if ( ( streq (stream, bios_get_stream_measurements())) ||
                 ( streq (stream, bios_get_stream_main()) ) ) {
                // New measurements publish
                std::string topic = bios_agent_subject(client);
                persist::process_measurement(topic, &in, c);
            }
            else {
                log_warning("Unsupported stream '%s' for STREAM DELIVER", stream);
            }
        }

        else {
                log_warning("Unsupported command '%s'", command);
        }
        ymsg_destroy (&in);
    }

    bios_agent_destroy (&client);
    log_close();
    return 0;
}
