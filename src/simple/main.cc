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

static const utils::Argv args{"./netmon"};
static utils::SubProcess netmon_proc{args};

void persistence_actor(zsock_t *pipe, void *args) {

    log_info ("%s", "persistence_actor() start\n");

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

        zmsg_t *msg = zmsg_recv(insock);

        try {
            bool b = utils::db::process_message (url, msg);
        } catch (tntdb::Error &e) {
            fprintf (stderr, "%s", e.what());
            fprintf (stderr, "%To resolve this problem, please see README file\n");
            log_critical ("%s: %s\n", "tntdb::Error caught", e.what());
            break;
        }
        zmsg_destroy(&msg);
    }
    
    zpoller_destroy(&poller);
    zsock_destroy(&insock);
    log_info ("%s", "persistence_actor end\n");
}
 
void netmon_actor(zsock_t *pipe, void *args) {

    log_info ("%s", "netmon_actor() start\n");

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
    log_info ("%s", "netmon_actor end\n");

}

void term_netmon(void) {
    if (netmon_proc.isRunning()) {
        netmon_proc.terminate();
    }
}

int main(int argc, char **argv) {

    log_open();
    log_set_level(LOG_DEBUG);

    srandom ((unsigned) time (NULL));
    bool test_mode = false;

    if (argc > 1 && strcmp(argv[1], "--test-mode") == 0) {
        test_mode = true;
        log_info ("%s", "Test Mode: Running mocked netmon_actor instead of netmon.\n");
    }

    atexit(term_netmon);

    zactor_t *db = zactor_new (persistence_actor, NULL);
    assert(db);
    zclock_sleep (2000);

    zactor_t *netmon = NULL;

    if (test_mode) {
        netmon = zactor_new (netmon_actor, NULL);
        assert(netmon);
    } else {
        netmon_proc.run();
        zclock_sleep(100);  //process handling is tricky - this ensures child has been started

        netmon_proc.poll();
        if (!netmon_proc.isRunning()) {
            fprintf(stderr, "ERROR: netmon does not run, exitcode: %d\n", netmon_proc.getReturnCode());
            return netmon_proc.getReturnCode();
        }
    }

    zactor_t *nut = zactor_new( nut_actor, NULL);
    assert(nut);
    
    zpoller_t *poller = zpoller_new(netmon, db, nut, NULL);
    assert(poller);

    while (!zpoller_terminated(poller)) {
        zsock_t *which = (zsock_t *)zpoller_wait (poller, -1);
    }
    
    if (test_mode) {    
        zactor_destroy(&netmon);        
        log_info ("%s", "destroying netmon_actor\n"); 
    }
    else {
        // normally kill() would be enough, but netmon can't cope
        // with them atm
        netmon_proc.terminate();
    }
    zactor_destroy(&nut);
    zactor_destroy(&db);
    log_info ("%s", "destroying persistence_actor\n"); 
    log_close ();
    return EXIT_SUCCESS;
}

