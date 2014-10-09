#include <string>
#include "nut-driver.h"
#include "defs.h"

void nut_actor(zsock_t *pipe, void *args) {
    zsock_t * dbsock = zsock_new_dealer(DB_SOCK);
    assert(dbsock);
    zpoller_t *poller = zpoller_new(pipe, NULL);
    assert(poller);
    zsock_signal(pipe, 0);
    UPSList listOfUPS;

    while(!zpoller_terminated(poller)) {
        zsock_t *which = (zsock_t *)zpoller_wait(poller, 5000);
        if (which == pipe) {
            break;
        } else if( which == NULL ) {
            listOfUPS.update();
            for(auto it = listOfUPS.begin(); it != listOfUPS.end() ; ++it) {
                if(it->second.changed() ) {
                    zmsg_t *msg = zmsg_new();
                    std::string id = "ups.status." + it->second.name();
                    zmsg_pushstr(msg, it->second.statusMessage().c_str() );
                    zmsg_pushstr(msg, id.c_str());
                    zmsg_send(&msg,dbsock);
                    zmsg_destroy(&msg);
                }
            }
        }
    }
    zpoller_destroy(&poller);
    zsock_destroy(&dbsock);
}
