/*    
    dshell - bios debug shell for malamute

    Idea and code skeleton taken from malamute/mshell.

SYNOPSIS:
    dshell endpoint timeout address stream pattern    -- show all matching messages

FUTURE:
    dshell endpoint timeout address stream subject body     -- send one message to this stream / subject
*/
#include <cstdlib>
#include <string>

#include <malamute.h>

#include "asset_msg.h"
#include "common_msg.h"
#include "netdisc_msg.h"
#include "nmap_msg.h"
#include "powerdev_msg.h"

void print_usage() {
    // TODO
}

int main (int argc, char *argv [])
{
    int argn = 1;
    bool verbose = false;
    if (argc > argn && streq (argv [argn], "-v")) {
        verbose = true;
        argn++;
    }
    // address has a special meaning for '/' character:
    // it delimits the password for authentication   
    char *endpoint  = (argn < argc) ? argv[argn++] : NULL;
    char *timeout   = (argn < argc) ? argv[argn++] : NULL;
    char *address   = (argn < argc) ? argv[argn++] : NULL;
    char *stream    = (argn < argc) ? argv[argn++] : NULL;
    char *subject   = (argn < argc) ? argv[argn++] : NULL;

    if (!endpoint || !timeout || !address || !stream || !subject || streq (stream, "-h")) {
        printf ("syntax: netmon_shell [-v] endpoint timeout address stream subject\n");
        printf ("\tsubject - regexp\n");
        return 0;
    }

    int timeout_num = 0;
    try {    
        timeout_num = std::stoi (timeout);
    } catch (std::exception &e) {
        printf ("Could not convert timeout to number.\n");
        return EXIT_SUCCESS;
    }
    if (timeout_num < 0) {
        printf ("timeout has to be a positive integer value.\n");
        return EXIT_SUCCESS;
    }

    mlm_client_verbose = verbose;
    mlm_client_t *client = mlm_client_new ();
    if (!client || mlm_client_connect(client, endpoint, timeout_num, address)) {
        zsys_error ("dshell: server not reachable at %s\n", endpoint);
        return 0;
    }
    
    //  Consume the event subjects specified by the pattern
    mlm_client_set_consumer (client, stream, subject);
    while (true) {
        //  Now receive and print any messages we get
        zmsg_t *msg = mlm_client_recv (client);
        if (!msg) {
            zsys_error ("interrupted\n");
            break;
        }
        printf ("sender=%s subject=%s content=",
                mlm_client_sender (client), mlm_client_subject (client));
        
        if (is_netdisc_msg (msg)) {
            printf ("\n");
            netdisc_msg_t *netdisc_msg = netdisc_msg_decode (&msg);
            netdisc_msg_print (netdisc_msg);
            netdisc_msg_destroy (&netdisc_msg);
        }
        else if (is_common_msg (msg)) {
            printf ("\n");
            common_msg_t *common_msg = common_msg_decode (&msg);
            common_msg_print (common_msg); // TODO: For some common messages it might make sense not use generic _print function
            common_msg_destroy (&common_msg);
        }
        else if (is_asset_msg (msg)) {
            printf ("\n");
            asset_msg_t *asset_msg = asset_msg_decode (&msg);
            asset_msg_print (asset_msg);
            asset_msg_destroy (&asset_msg);
            // TODO: For some messages it makes sense to not use generic _print function
        }
        else if (is_nmap_msg (msg)) {
            printf ("\n");
            nmap_msg_t *nmap_msg = nmap_msg_decode (&msg);
            nmap_msg_print (nmap_msg);
            nmap_msg_destroy (&nmap_msg);
            // TODO: For some messages it makes sense to not use generic _print function
        }
        else if (is_powerdev_msg (msg)) {
            printf ("\n");
            powerdev_msg_t *powerdev_msg = powerdev_msg_decode (&msg);
            powerdev_msg_print (powerdev_msg);
            powerdev_msg_destroy (&powerdev_msg);
            // TODO: For some messages it makes sense to not use generic _print function
        }
        else { // assume string
            char *content = zmsg_popstr (msg);
            printf ("%s\n", content ? content : "<nullptr>");
            zstr_free (&content);
        }
        zmsg_destroy (&msg);
    }

    mlm_client_destroy (&client);
    return EXIT_SUCCESS;
}
