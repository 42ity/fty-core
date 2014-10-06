#include "persistence.h"
#include "dbinit.h"
#include "defs.h"
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>

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
        if (which == insock)
        {
 
            zmsg_t *msg = zmsg_recv(insock);
            zmsg_pop(msg);  // remove routing info
            char *msg_header = zmsg_popstr (msg);
            
            int n = 0; // number of rows affected
            if ( strcmp(msg_header ,"netmon") == 0 )
            {
                char *prefixlen = zmsg_popstr(msg);
                char *mac = zmsg_popstr(msg);
                char *ipaddr = zmsg_popstr(msg);
                char *ipver = zmsg_popstr(msg);
                char *name = zmsg_popstr(msg);
                char *command = zmsg_popstr(msg);
                
                //drop : from string and reduce to apropriate length
                std::string s(mac);
                std::remove( s.begin(), s.end(), ':');
                s.erase(12,5);
                int mask = atoi(prefixlen);

                utils::NetHistory nethistory = utils::NetHistory(url);

                nethistory.setMac(s);
                nethistory.setName(name);
                nethistory.setIp(ipaddr);
                nethistory.setMask(mask);
                nethistory.setCommand(*command);
            
                int n = nethistory.dbsave();

                zstr_free(&command);
                zstr_free(&name);
                zstr_free(&ipver);
                zstr_free(&ipaddr);
                zstr_free(&mac);
                zstr_free(&prefixlen);
            }
            else if (  strcmp(msg_header ,"nmap") == 0  )
            {
            }
            else if (  strcmp(msg_header ,"nut") == 0  )
            {
            }
            //TODO add all clients for message parsing
            //else if (  strcmp(msg_header ,"nmap") == 0  )
            //else if (  strcmp(msg_header ,"nmap") == 0  )

//        if ( n == 1 )
//        {   // success
//
//        }
//        else
//        {   // fail
//
//        }
//        
            zstr_free(&msg_header);          
            zmsg_destroy(&msg);
            i++;
        }


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
    int prefixlen = 8;
    const char *mac = "01:23:45:67:89:ab";
    const char *command = "a";
     
    while(!zpoller_terminated(poller)) {

        zsock_t *which = (zsock_t *) zpoller_wait(poller, 1000);
        if (which == pipe) {
                break;
        }
        
        zmsg_t * msg = zmsg_new();
        
        //add all necessary fields
        zmsg_pushstr(msg, command);
        zmsg_pushstr(msg, name);
        zmsg_pushstr(msg, ipver);
        zmsg_pushstr(msg, ipaddr);
        zmsg_pushstr(msg, mac);
        //  convert int to char
        char mask[4];
        sprintf(mask,"%d",prefixlen);
        zmsg_pushstrf(msg,mask);
        //add header
        zmsg_pushstr (msg, "netmon");
        zmsg_send(&msg, dbsock);
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
