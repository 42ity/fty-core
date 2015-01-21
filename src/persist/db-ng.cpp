#include "malamute.h"
#include "measure_types.h"
#include "monitor.h"
#include "persistencelogic.h"

#include <stdio.h>
#include <zsys.h>

int main (int argc, char *argv []) {
    // Basic settings
    if (argc > 1) {
        printf ("syntax: server-agent [ ipc://...|tcp://... ]\n");
        return 0;
    }
    const char *addr = (argc == 1) ? "ipc://@/malamute" : argv[1];

    // Create a client
    mlm_client_t *client = mlm_client_new(addr, 1000, "persistence");
    if (!client) {
        zsys_error ("server-agent: server not reachable at ipc://@/malamute");
        return 0;
    }

    // We are listening for measurements
    mlm_client_set_consumer(client, "measurements", ".*");
    while(!zsys_interrupted) {
        zmsg_t *msg = mlm_client_recv(client);
        if(msg == NULL)
            break;
        printf("Command is %s\n", mlm_client_command(client));
        // Mailbox deliver is direct message
        if(streq (mlm_client_command(client), "MAILBOX DELIVER")) {
            // Verify it is for us
            if(!streq(mlm_client_subject(client), "persistence"))
                continue;

            // Currently only measures meta is supported
            if(!is_common_msg(msg))
                continue;
            zmsg_t *rep = persist::process_message(&msg);
            if(rep != NULL) {
                // Send a reply back
                mlm_client_sendto(client, (char*)mlm_client_sender(client),
                                        "persistence", NULL, 0, &rep);
            } else
                continue;
        // Other option is stream aka publish subscribe
        } else {
            persist::process_message(&msg);
        }
        zmsg_destroy(&msg);
    }

    mlm_client_destroy (&client);
    return 0;
}
