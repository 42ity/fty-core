#include "cleanup.h"
#include "malamute.h"
#include "monitor.h"
#include "ymsg.h"
#include "bios_agent.h"
#include "persistencelogic.h"
#include "dbpath.h"
#include "log.h"
#include "defs.h"
#include "str_defs.h"

#include <stdio.h>
#include <zsys.h>

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

    bios_agent_set_consumer(client, bios_get_stream_main (), ".*");
    while(!zsys_interrupted) {
        _scoped_ymsg_t *in = bios_agent_recv(client);
        if ( in == NULL )
            continue;
        log_info ("Command is '%s'", bios_agent_command(client));
        // Mailbox deliver is direct message
        if(streq (bios_agent_command(client), "MAILBOX DELIVER"))
        {
            if(ymsg_id(in) != YMSG_SEND) {
                log_warning("Got REPLY ymsg, that looks weird, discarding!\n");
                ymsg_destroy(&in);
                continue;
            }
            _scoped_ymsg_t *out = NULL;
            char* out_subj = NULL;
            persist::process_ymsg(&out, &out_subj, in, bios_agent_subject(client));
            if ( ( out_subj == NULL ) || ( out == NULL ) ){
                ymsg_destroy(&in);
                ymsg_destroy(&out);
                continue;
            }
            bios_agent_replyto(client, (char*)bios_agent_sender(client), out_subj, &out, in );
            ymsg_destroy(&out);
            free(out_subj);
        }
        // Other option is stream aka publish subscribe
        else {
            // New measurements publish
            std::string topic = bios_agent_subject(client);
            persist::process_measurement(topic, &in);
        }
        ymsg_destroy (&in);
    }

    bios_agent_destroy (&client);
    log_close();
    return 0;
}
