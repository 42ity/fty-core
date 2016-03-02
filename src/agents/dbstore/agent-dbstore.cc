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

#include <cxxtools/regex.h>

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

static void
s_metric_store(zsock_t *pipe, void* args)
{
    char* endpoint = (char*) args;

    persist::TopicCache topic_cache{10*1024};
    std::list<std::string> m_cache;
    
    cxxtools::Regex warranty_subject{"^end_warranty_date@.*$"};

    mlm_client_t *client = mlm_client_new ();
    mlm_client_connect (client, endpoint, 1000, "metric-store");
    mlm_client_set_consumer(client, "METRICS", ".*");

    zpoller_t *poller = zpoller_new (pipe, mlm_client_msgpipe (client), NULL);

    zsock_signal (pipe, 0);

    while (!zsys_interrupted) {

        void * which = zpoller_wait (poller, -1);

        if (!which || which == pipe)
            break;

        zmsg_t *msg = mlm_client_recv (client);

        if (warranty_subject.match (mlm_client_subject (client))) {
            zmsg_destroy (&msg);
            continue;
        }

        persist::process_measurement(&msg, topic_cache,m_cache,1);

        zmsg_destroy (&msg);
    }

    zpoller_destroy (&poller);
    mlm_client_destroy (&client);
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
    const char *addr = (argc == 1) ? MLM_ENDPOINT : argv[1];
    if (argc > 2) {
        url = argv[2];
    }

    // create actor storing measurements
    zactor_t *actor = zactor_new (s_metric_store, (void*) addr);

    // Create a client
    bios_agent_t *client = bios_agent_new(addr, BIOS_AGENT_NAME_DB_MEASUREMENT);
    if(!client) {
        log_error ("agent-dbstore: error bios_agent_new ()");
        return 1;
    }

    bios_agent_set_consumer(client, bios_get_stream_main(), ".*");
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

            if (strncmp(bios_agent_subject(client), "inventory", 9) == 0 ) {
                log_debug ("inventory message recieved, ignoring it. In future this should never happen");
            }
            else if (strncmp(bios_agent_subject(client), "configure", 9) == 0 ) {
                log_debug ("configure message recieved, ignoring it. In future this should never happen");
            }
            else {
                /* TODO: Previously everything not specified above was
                 * ignored silently. Now it is loud. No idea if anything
                 * SHOULD be done about such messages. //Jim 2016-01-26 */
                log_debug ("%s STREAM DELIVERed message recieved, ignoring it", bios_agent_subject(client));
            }
        }

        else {
                log_warning("Unsupported command '%s'", command);
        }
        ymsg_destroy (&in);
    }

    zactor_destroy (&actor);
    bios_agent_destroy (&client);
    return 0;
}
