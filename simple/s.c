#include "persistence.h"
#include "dbinit.h"
#include "defs.h"
#include <algorithm>

#define MSG_T_NETMON  1

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

        
        char *name; 
        char *ipver;
        char *ipaddr;
        char *prefixlen;
        char *mac;
        char *unknown; // TODO because we use router/dealer there appears one more frame with empty string.
        char *command;
        int mask;
 
        //zmsg_t *msg = zmsg_new();

        zstr_recvx(insock, &unknown,&name,&ipver,&ipaddr,&mac,&command,&prefixlen,NULL);

//        zmsg_pop(msg);  // remove routing info
//        zmsg_pushstrf(msg, "%d: PERSISTENCE got ", i);
//        zmsg_pushstrf(msg,"name = %s", name);
//        zmsg_pushstrf(msg,"ipver = %s", ipver);
//        zmsg_pushstrf(msg,"ipaddr = %s", ipaddr);
//        zmsg_pushstrf(msg,"mac = %s",mac);
//        zmsg_pushstrf(msg,"unknown = %s",unknown);
//
        int n ; // number of rows inserted;
        int msgtype = MSG_T_NETMON;
        std::string s(mac);
        std::remove( s.begin(), s.end(), ':');

        switch (msgtype) {

            case MSG_T_NETMON:
            {
                mask = atoi(prefixlen);

                utils::NetHistory nethistory = utils::NetHistory(url);

                nethistory.setMac(s);
                nethistory.setName(name);
                nethistory.setIp(ipaddr);
                nethistory.setMask(mask);
                nethistory.setCommand(*command);
        
                n = nethistory.dbsave();

                break;
            }
            default:
                break;
        }
//        if ( n == 1 )
//        {   // success
//
//        }
//        else
//        {   // fail
//
//        }
//        
        zstr_free(&name);
        zstr_free(&ipver);
        zstr_free(&ipaddr);
        zstr_free(&mac);
        zstr_free(&unknown);
        zstr_free(&command);
        zstr_free(&prefixlen);
//        zmsg_print(msg);
//        zmsg_destroy(&msg);
        i++;

    }

    zpoller_destroy(&poller);
    zsock_destroy(&insock);

}
 
void netmon_actor(zsock_t *pipe, void *args) {

    zsock_t * dbsock = zsock_new_dealer(DB_SOCK);
    assert(dbsock);
    zsock_t * nlsock = zsock_new_dealer(NETLOGIC_SOCK);
    assert(nlsock);
    zpoller_t *poller = zpoller_new(pipe, NULL);
    assert(poller);
    zsock_signal(pipe, 0);
 
    size_t i = 0;
    int timeout = -1;

    const char *name = "myname";
    const char *ipver = "4";
    const char *ipaddr = "10.144.55.5";
    const char *prefixlen = "8";
    const char *mac = "0123456789ab";
    const char *command = "a";
     
    while(!zpoller_terminated(poller)) {

        zsock_t *which = (zsock_t *) zpoller_wait(poller, 1000);
        if (which == pipe) {
                break;
        }
        zstr_sendx(dbsock, "aaa",ipver,ipaddr,mac,command,prefixlen,NULL); 
        //zstr_sendf(dbsock, "%d: hello", i);
        //zstr_sendf(nlsock, "%d: hello", i);
        i++;
    }

    zpoller_destroy(&poller);
    zsock_destroy(&nlsock);
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
