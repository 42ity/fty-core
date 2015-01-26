#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <algorithm>
#include <vector>
#include <exception>
#include <czmq.h>
#include <tntdb/error.h>

#include "cidr.h"
#include "persistence.h"
#include "persistencelogic.h"
#include "dbinit.h"
#include "defs.h"
#include "log.h"
#include "netdisc_msg.h"
#include "subprocess.h"
#include "nut-actor.h"

#define MSG_T_NETMON  1
#define randof(num)  (int) ((float) (num) * random () / (RAND_MAX + 1.0))

// netmon child process
static const shared::Argv netmon_args{"./netmon"};
static shared::SubProcess netmon_proc{netmon_args, false, false};
// nmap scanner child process
static const shared::Argv nmap_args{"./driver-nmap"};
static shared::SubProcess nmap_proc{nmap_args, false, false};

//  netmon 
void filip_actor (zsock_t *pipe, UNUSED_PARAM void *args) {
    log_info ("start");

    zsock_t * incoming = zsock_new_router (FILIP_SOCK);
    assert (incoming);

    zsock_t * dbsock = zsock_new_dealer (DB_SOCK);
    assert (dbsock);

    zsock_t * nmap  = zsock_new_dealer (DRIVER_NMAP_SOCK);
    assert (nmap);

    zsock_t * nmap_reply = zsock_new_router (DRIVER_NMAP_REPLY);
    assert (nmap_reply);

    zpoller_t *poller = zpoller_new (incoming, nmap_reply, pipe, NULL);
    assert (poller);

    zsock_signal(pipe, 0);

    while(!zpoller_terminated(poller)) {
        zsock_t *which = (zsock_t *) zpoller_wait(poller, -1);
        if (which == pipe) {
            break;
        }
        else if (which == incoming) {
            log_debug ("socket INCOMING received a message");    
            netdisc_msg_t *msg = netdisc_msg_recv (incoming);  
            assert (msg);
            if (netdisc_msg_ipaddr (msg) == NULL) {
                    log_warning ("Field 'ipaddr' is empty!");
                    netdisc_msg_destroy (&msg);
                    continue;
            }
            shared::CIDRAddress addr (netdisc_msg_ipaddr (msg),
                                      std::to_string (netdisc_msg_prefixlen (msg)));
            shared::CIDRAddress newaddr = addr.network ();
            std::string target (newaddr.toString (shared::CIDROptions::CIDR_WITHOUT_PREFIX));
            target.append ("/");
            target.append (std::to_string (newaddr.prefix ()));

            // construct the nmap scan_command
            nmap_msg_t * scan_command = nmap_msg_new (NMAP_MSG_SCAN_COMMAND);
            assert (scan_command);
            nmap_msg_set_type (scan_command, "%s", "defaultlistscan", NULL);

            zlist_t *zl = zlist_new ();
            zlist_append (zl, (void *) target.c_str());
            nmap_msg_set_args (scan_command, &zl);
            zlist_destroy (&zl);

            // send 
            nmap_msg_send (&scan_command, nmap); 

            netdisc_msg_destroy (&msg);
        }
        else if (which == nmap_reply) {
            log_debug ("socket NMAP_REPLY received a message");    
            nmap_msg_t *msg = nmap_msg_recv (nmap_reply);
            assert (msg);
            nmap_msg_send (&msg, dbsock);
            assert (msg == NULL);
        }
    }

    zpoller_destroy (&poller);
    zsock_destroy (&incoming);
    zsock_destroy (&dbsock);
    zsock_destroy (&nmap);
    zsock_destroy (&nmap_reply);

    log_info ("end");
}

void persistence_actor(zsock_t *pipe, UNUSED_PARAM void *args) {
    log_info ("start");

    zsock_t * insock = zsock_new_router(DB_SOCK);
    assert(insock);

    zpoller_t *poller = zpoller_new(insock, pipe, NULL);
    assert(poller);

    zsock_signal(pipe, 0);

    while(!zpoller_terminated(poller)) {
        zsock_t *which = (zsock_t *) zpoller_wait(poller, -1);
        if (which == pipe) {
            break;
        }
        log_debug ("message received");
        zmsg_t *msg = zmsg_recv(insock);

        try {
            bool b = persist::process_message (url, msg);
	    if (!b)
    		log_debug ("message processing returned false");
        } catch (tntdb::Error &e) {
            fprintf (stderr, "%s", e.what());
            fprintf (stderr, "To resolve this problem, please see README file\n");
            log_critical ("%s: %s", "tntdb::Error caught", e.what());
            break;
        }
        zmsg_destroy(&msg);
    }
    
    zpoller_destroy(&poller);
    zsock_destroy(&insock);
    log_info ("end");
}
 
void netmon_actor(zsock_t *pipe, UNUSED_PARAM void *args) {

    log_info ("start");

    const int names_len = 6;
    const char *names[6] = { "eth0", "eth1", "enps02", "wlan0", "veth1", "virbr0" };

    zsock_t * dbsock = zsock_new_dealer (DB_SOCK);
    assert(dbsock);
    zpoller_t *poller = zpoller_new (dbsock, pipe, NULL);
    assert(poller);

    zsock_signal(pipe, 0);

    // Until SIGINT, randomly select between addition or deletion event
    // if it's addition - generate some random data (name, ipver...), add this to a vector; send message
    // if it's deletion - and vector not empty: select one randomly from vector, remove from vector, send message; else skip
    // sleep for (800, 2000) ms
    std::vector<std::tuple<std::string, byte, std::string, byte, std::string>> stored;         
    while(!zpoller_terminated (poller)) {        
        zsock_t *which = (zsock_t *) zpoller_wait(poller, randof(1200) + 800);
        if (which == pipe) {
                break;
        }

        byte command = random() % 2; // 0 - os_add; 1 - os_del
        if (command == 0) {            
            netdisc_msg_t *ndmsg = netdisc_msg_new (NETDISC_MSG_AUTO_ADD);
            netdisc_msg_set_name (ndmsg, "%s", names[randof(names_len)]);
            netdisc_msg_set_ipver (ndmsg, 0);
            netdisc_msg_set_ipaddr (ndmsg, "%d.%d.%d.%d",
                randof(255) + 1, randof(256), randof(256), randof(256));
            netdisc_msg_set_prefixlen (ndmsg, (byte) randof(33));
            netdisc_msg_set_mac (ndmsg, "%s", "bb:0a:21:02:fe:aa");
           // store it
            stored.push_back (
                std::make_tuple (netdisc_msg_name (ndmsg),
                                 netdisc_msg_ipver (ndmsg),
                                 netdisc_msg_ipaddr (ndmsg),
                                 netdisc_msg_prefixlen (ndmsg),
                                 netdisc_msg_mac (ndmsg)));
            // send it
            netdisc_msg_send (&ndmsg, dbsock);          
 
        }
        else if (command == 1) {
            if (stored.size() == 0)
                continue;
            int which = randof(stored.size());

            netdisc_msg_t *ndmsg = netdisc_msg_new (NETDISC_MSG_AUTO_DEL);
            netdisc_msg_set_name (ndmsg, "%s", std::get<0>(stored.at(which)).c_str());
            netdisc_msg_set_ipver (ndmsg, 0);
            netdisc_msg_set_ipaddr (ndmsg, "%s",  std::get<2>(stored.at(which)).c_str());
            netdisc_msg_set_prefixlen (ndmsg, (byte) std::get<3>(stored.at(which)));
            netdisc_msg_set_mac (ndmsg, "%s", std::get<4>(stored.at(which)).c_str());            
            // send it
            netdisc_msg_send (&ndmsg, dbsock);            
            // erase
            auto it = stored.begin();
            for (int i = 0; i < which; ++i) { ++it; }             
            stored.erase (it);
        }
    }

    zsock_destroy(&dbsock);
    log_info ("end");

}

void term_children (void) {
    if (netmon_proc.isRunning()) {
        netmon_proc.terminate();
    }
    if (nmap_proc.isRunning()) {
        nmap_proc.terminate();
    }
}

int main(int argc, char **argv) {

    log_open();
    log_set_level(LOG_DEBUG);

    log_info ("==========================================");
    log_info ("==========        SIMPLE        ==========");

    srandom ((unsigned) time (NULL));
    bool test_mode = false;

    if (argc > 1 && strcmp(argv[1], "--test-mode") == 0) {
        test_mode = true;
        log_info ("= %s =", "TEST MODE: Running netmon_actor() instead of netmon.");
    }
    log_info ("==========================================");

    atexit(term_children);

    zactor_t *db = zactor_new (persistence_actor, NULL);
    assert(db);
    zclock_sleep (2000);

    zactor_t *filip = zactor_new (filip_actor, NULL);
    assert(filip);
    zclock_sleep (1000);

    zactor_t *netmon = NULL;

    if (test_mode) {
        netmon = zactor_new (netmon_actor, NULL);
        assert(netmon);
    } else {
        // nmap
        nmap_proc.run();
        zclock_sleep(100);  //process handling is tricky - this ensures child has been started

        nmap_proc.poll();
        if (!nmap_proc.isRunning()) {
            log_error ("driver-nmap did not start, exitcode: '%d'", nmap_proc.getReturnCode());
            return nmap_proc.getReturnCode();
        }
        else {
            log_info ("driver-nmap is running");
        }
           
        // netmon
        netmon_proc.run();
        zclock_sleep(100);  //process handling is tricky - this ensures child has been started

        netmon_proc.poll();
        if (!netmon_proc.isRunning()) {
            log_error ("netmon did not start; exitcode: '%d'", netmon_proc.getReturnCode());
            return netmon_proc.getReturnCode();
        }
        else {
            log_info ("netmon is running");
        }
     }

    zactor_t *nut = zactor_new (drivers::nut::nut_actor, NULL);
    assert(nut);

    zpoller_t *poller = zpoller_new(netmon, db, nut, filip, NULL);
    assert(poller);

    while (!zpoller_terminated(poller)) {
        zsock_t *which = (zsock_t *)zpoller_wait (poller, -1);
	// FIXME: JIM: Use the variable somehow to avout unused warning
	if (which==NULL)
	    log_info ("zpoller_wait() timed out or was aborted");
    }

    if (test_mode) {    
        zactor_destroy(&netmon);        
        log_info ("%s", "destroying netmon_actor"); 
    }
    else {
        // normally kill() would be enough, but netmon can't cope
        // with them atm
        netmon_proc.terminate();
        nmap_proc.terminate();
    }
    zactor_destroy(&nut);
    zactor_destroy(&db);
    zactor_destroy(&filip);
    log_info ("END"); 
    log_close ();
    return EXIT_SUCCESS;
}

