#include "malamute.h"
#include "measure_types.h"
#include "monitor.h"
#include "bios_agent.h"
#include "persistencelogic.h"
#include "dbpath.h"
#include "log.h"

#include <stdio.h>
#include <zsys.h>

int main (int argc, char *argv []) {
    
    log_open();
    log_info ("persistence.measurement started");
    
    // Basic settings
    if (argc > 3) {
        printf ("syntax: db-ng [ <endpoint> | <endpoint> <mysql:db=bios;user=bios;password=test> ]\n");
        return 1;
    }
    const char *addr = (argc == 1) ? "ipc://@/malamute" : argv[1];
    if (argc > 2) {
        url = argv[2];
    }

    // Create a client
    mlm_client_t *client = mlm_client_new();
    if(!client) {
        log_error ("db-ng: error mlm_client_new");
        return 1;
    }
    if(mlm_client_connect(client, addr, 1000, "persistence.measurement") != 0) {
        log_error ("db-ng: server not reachable at '%s'", addr);
        return 1;
    }

    // We are listening for measurements
    mlm_client_set_consumer(client, "measurements", ".*");
    while(!zsys_interrupted) {
        zmsg_t *msg = mlm_client_recv(client);
        if(msg == NULL)
            continue;
        log_info ("Command is '%s'", mlm_client_command(client));
        // Mailbox deliver is direct message
        if(streq (mlm_client_command(client), "MAILBOX DELIVER")) {
            // Verify it is for us
            if(!streq(mlm_client_subject(client), "persistence")) {
                log_warning("Got direct message with wrong subject, discarding!\n");
                continue;
            }

            zmsg_t *rep = persist::process_message(&msg);
            if(rep != NULL) {
                // Send a reply back
                mlm_client_sendto(client, (char*)mlm_client_sender(client),
                                        "persistence", NULL, 0, &rep);
            } else
                continue;
        // Other option is stream aka publish subscribe
        } else {
            // New measurements publish
            if(is_ymsg(msg)) {
                std::string topic = mlm_client_subject(client);
                persist::process_measurement(topic, &msg);
            // Legacy stuff
            } else {
                zmsg_t *rep = persist::process_message(&msg);
                if(rep != NULL)
                    zmsg_destroy(&rep);
            }
        }
        zmsg_destroy(&msg);
    }

    mlm_client_destroy (&client);
    log_close();
    return 0;
}
