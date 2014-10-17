#include <string>
#include "nut-driver.h"
#include "powerdev_msg.h"
#include "defs.h"
#include "log.h"

// basic powerdev properties
static const std::vector<std::string> basicProperties {
    "model",
    "manufacturer",
    "serial",
    "type",
    "status",
};

zmsg_t *nutdevice_to_powerdev_msg(const NUTDevice &dev) {
    zmsg_t *msg;
    char *name[basicProperties.size()];

    zhash_t *otherProperties = zhash_new();
    assert(otherProperties);
    auto properties = dev.properties();

    memset(name,0, sizeof(char *) * basicProperties.size() );
    // get basics properties
    for(unsigned int i=0; i < basicProperties.size(); ++i) {
        auto iter = properties.find(basicProperties[i]);
        if( iter != properties.end() ) {
            name[i] = strdup(iter->second.c_str());
            properties.erase(iter);
        }
    }
    // get extra properties
    for( auto &iter : properties ) {
        zhash_insert(otherProperties, iter.first.c_str(), (void *)( iter.second.c_str() ) );
    }
    msg = powerdev_msg_encode_powerdev_status(
         dev.name().c_str(),
         name[0],
         name[1],
         name[2],
         name[3],
         name[4],
         otherProperties
    );
    // clean allocated memory
    for(unsigned int i=0; i < basicProperties.size(); ++i) {
        if(name[i]) free(name[i]);
    }
    zhash_destroy(&otherProperties);
    return msg;
}

void nut_actor(zsock_t *pipe, void *args) {
    log_info ("%s", "nut_actor start\n");

    zsock_t * dbsock = zsock_new_dealer(DB_SOCK);
    assert(dbsock);
    zpoller_t *poller = zpoller_new(pipe, NULL);
    assert(poller);
    zsock_signal(pipe, 0);
    NUTDeviceList listOfUPS;

    while(!zpoller_terminated(poller)) {
        zsock_t *which = (zsock_t *)zpoller_wait(poller, 1000);
        if (which == pipe) {
            break;
        } else if( which == NULL ) {
            listOfUPS.update();
            for(auto it = listOfUPS.begin(); it != listOfUPS.end() ; ++it) {
                if(it->second.changed() ) {
                    zmsg_t *msg = nutdevice_to_powerdev_msg(it->second);
                    log_info ("ups %s : %s\n", it->second.name().c_str(), it->second.toString().c_str() );
                    zmsg_send(&msg,dbsock);
                    zmsg_destroy(&msg);
                    it->second.changed(false);
                }
            }
        }
    }
    zpoller_destroy(&poller);
    zsock_destroy(&dbsock);
    log_info ("%s", "nut_actor end\n");
}
