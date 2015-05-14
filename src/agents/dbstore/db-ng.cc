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
    if(mlm_client_connect(client, addr, 1000, BIOS_AGENT_NAME_DB_MEASUREMENT) != 0) {
        log_error ("db-ng: server not reachable at '%s'", addr);
        return 1;
    }

    // We are listening for measurements
    mlm_client_set_consumer(client, bios_get_stream_main (), ".*"); // regex might be "^measurements\..+" 
    while(!zsys_interrupted) {
        _scoped_zmsg_t *msg = mlm_client_recv(client);
        if(msg == NULL)
            continue;
        log_info ("Command is '%s'", mlm_client_command(client));
        // Mailbox deliver is direct message
        if(streq (mlm_client_command(client), "MAILBOX DELIVER")) {
            if(is_ymsg(msg)) {
                _scoped_ymsg_t *in = ymsg_decode(&msg);
                if(ymsg_id(in) != YMSG_SEND) {
                    log_warning("Got REPLY ymsg, that looks weird, discarding!\n");
                    ymsg_destroy(&in);
                    continue;
                }
                _scoped_ymsg_t *out = ymsg_new(YMSG_REPLY);
                char* out_subj = NULL;
                persist::process_ymsg(out, &out_subj, in, mlm_client_subject(client));
                if(out_subj == NULL) {
                    ymsg_destroy(&in);
                    ymsg_destroy(&out);
                    continue;
                }
                ymsg_set_version(out, ymsg_version(in));
                ymsg_set_seq(out, ymsg_seq(in));
                ymsg_set_rep(out, ymsg_seq(in));
                if(ymsg_is_repeat(in)) { // default is not to repeat
                    zchunk_t *chunk = ymsg_get_request(in);
                    ymsg_set_request(out, &chunk);
                }
                _scoped_zmsg_t *out_z = ymsg_encode(&out);
                if(out_z != NULL)
                mlm_client_sendto(client, (char*)mlm_client_sender(client),
                                          out_subj, NULL, 0, &out_z);
                ymsg_destroy(&in);
                ymsg_destroy(&out);
                zmsg_destroy(&out_z);
                free(out_subj);
            } else {
                // Verify it is for us
                if(!streq(mlm_client_subject(client), "persistence")) {
                    log_warning("Got direct message with wrong subject, discarding!\n");
                    continue;
                }

                _scoped_zmsg_t *rep = persist::process_message(&msg);
                if(rep != NULL) {
                    // Send a reply back
                    mlm_client_sendto(client, (char*)mlm_client_sender(client),
                                            "persistence", NULL, 0, &rep);
                } else
                    continue;
            }
        // Other option is stream aka publish subscribe
        } else {
            // New measurements publish
            if(is_ymsg(msg)) {
                std::string topic = mlm_client_subject(client);
                persist::process_measurement(topic, &msg);
            // Legacy stuff
            } else {
                _scoped_zmsg_t *rep = persist::process_message(&msg);
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
