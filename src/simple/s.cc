#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <stdio.h>

#include "persistence.h"
#include "dbinit.h"
#include "defs.h"
#include "netdisc_msg.h"

#define MSG_T_NETMON  1
#define randof(num)  (int) ((float) (num) * random () / (RAND_MAX + 1.0))


// ugly, but this is not production code
int irq = 0;
void interrupt (int signum) {
    irq = 1;
}

void persistence_actor(zsock_t *pipe, void *args) {

    zsock_t * insock = zsock_new_router(DB_SOCK);
    assert(insock);

    zpoller_t *poller = zpoller_new(insock, pipe, NULL);
    assert(poller);

    zsock_signal(pipe, 0);
 
    size_t i = 0;

    while(!zpoller_terminated(poller)) {

        zsock_t *which = (zsock_t *) zpoller_wait(poller, -1);
        if (which == pipe) {
            break;
        }

        netdisc_msg_t *msg = netdisc_msg_recv (insock);
        
        const char *name = netdisc_msg_name (msg); 
        byte ipver = netdisc_msg_ipver (msg);
        const char *ipaddr = netdisc_msg_ipaddr (msg);
        byte prefixlen = netdisc_msg_prefixlen (msg);
        const char *mac = netdisc_msg_mac (msg);
        int command_id = netdisc_msg_id (msg);
        char command;
        if (command_id == 1) {
            command = 'a';
        } else {
            command = 'd';
        }
        int mask;

        int n ; // number of rows inserted;
        std::string s(mac);
        std::remove( s.begin(), s.end(), ':');
        s.erase(12,5);

        // So far, only netmon messages are being stored
        utils::NetHistory nethistory = utils::NetHistory(url);

        puts (name);
        puts (ipaddr);
        nethistory.setMac(s); // mac
        nethistory.setName(name);
        nethistory.setIp(ipaddr);
        nethistory.setMask(prefixlen);
        nethistory.setCommand(command);
        
        n = nethistory.dbsave();

        i++;
        netdisc_msg_destroy (&msg);
    }

    }
    zpoller_destroy(&poller);
    zsock_destroy(&insock);
}
 
void netmon_actor(zsock_t *pipe, void *args) {

    const int names_len = 6;    
    const char *names[6] = { "eth0", "eth1", "enps02", "wlan0", "veth1", "virbr0" };     
    
    zsock_t * dbsock = zsock_new_dealer(DB_SOCK);
    assert(dbsock);

    // Until SIGINT, randomly select between addition or deletion event
    // if it's addition - generate some random data (name, ipver...), add this to a vector; send message
    // if it's deletion - and vector not empty: select one randomly from vector, remove from vector, send message; else skip
    // sleep for (800, 2000) ms
    std::vector<std::tuple<std::string, byte, std::string, byte, std::string>> stored;         
    while(1) {
        if (irq == 1)
            break;
        byte command = random() % 2; // 0 - os_add; 1 - os_del
        if (command == 0) {            
            netdisc_msg_t *ndmsg = netdisc_msg_new (NETDISC_MSG_OS_ADD);
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

            netdisc_msg_t *ndmsg = netdisc_msg_new (NETDISC_MSG_OS_DEL);
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
        zclock_sleep (randof(1200) + 800);
    }

    zsock_destroy(&dbsock);
}

void netlogic_actor(zsock_t *pipe, void *args) {

    zsock_t * insock = zsock_new_router(NETLOGIC_SOCK);
    assert(insock);
    zsock_t * dbsock = zsock_new_dealer(DB_SOCK);
    assert(dbsock);
    zpoller_t *poller = zpoller_new(insock, pipe, NULL);
    assert(poller);
    zsock_signal(pipe, 0);

    size_t i = 0;

    while(!zpoller_terminated(poller)) {

        zsock_t *which =  (zsock_t *)  zpoller_wait(poller, 1000);
        if (which == pipe) {
            break;
        }

        if (which == insock) {
            zmsg_t *msg = zmsg_recv(insock);
            zmsg_pop(msg);  // remove routing info
            zmsg_pushstrf(msg, "%d: NETLOGIC got ", i);
            zmsg_send(&msg, dbsock);
            i++;
            continue;
        }

    }

    zpoller_destroy(&poller);
    zsock_destroy(&insock);
    zsock_destroy(&dbsock);

}


int main() {
    srandom ((unsigned) time (NULL));
    zsys_catch_interrupts ();
    zsys_handler_set (&interrupt);

    int i;

    zactor_t *db = zactor_new(persistence_actor, NULL);
    assert(db);

    zactor_t *netmon = zactor_new(netmon_actor, NULL);
    assert(netmon);
    
    zactor_t *netlogic = zactor_new(netlogic_actor, NULL);
    assert(netlogic);

    zsock_t *cli_sock = zsock_new_rep(CLI_SOCK);
    assert(cli_sock);

    zpoller_t *poller = zpoller_new(netmon, cli_sock, NULL);
    assert(poller);

    while (!zpoller_terminated(poller)) {
        zsock_t *which =  (zsock_t *) zpoller_wait(poller, -1);

        if (which == cli_sock) {
            //handle command line
            char* cmd = zstr_recv(cli_sock);
            fprintf(stderr, "INFO: got event on cli_sock, cmd: '%s'\n", cmd);
            zstr_send(netmon, cmd);
            char* reply = zstr_recv(netmon);
            zstr_send(cli_sock, reply);
            char* exp_reply;
            asprintf(&exp_reply, "%s/ACK", cmd);
            bool to_exit = false;
            if (!streq(reply, exp_reply)) {
                fprintf(stderr, "ERROR: invalid reply, expected '%s', got '%s'\n", exp_reply, reply);
                zstr_send(netmon, "$TERM");
                to_exit = true;
            } else if (streq(cmd, "quit")) {
                to_exit = true;
            }
            free(exp_reply);
            free(reply);
            free(cmd);
            if (to_exit) {
                fprintf(stderr, "INFO: exiting\n");
                break;
            }
        } else if (which == zactor_resolve(netmon)) {
            fprintf(stderr, "INFO: got unexpected message from netmon");
            zmsg_t *msg = zmsg_recv(netmon);
            zmsg_print(msg);
            zmsg_destroy(&msg);
        }

    }

    zpoller_destroy(&poller);
    zsock_destroy(&cli_sock);
    zactor_destroy(&netlogic);
    zactor_destroy(&netmon);
    zactor_destroy(&db);

}
