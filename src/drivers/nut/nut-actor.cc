#include <string>
#include <iostream>
#include "nut-driver.h"
#include "powerdev_msg.h"
#include "measure_types.h"
#include "common_msg.h"
#include "defs.h"
#include "log.h"

namespace drivers
{
namespace nut
{

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
        zhash_insert(otherProperties, (void*)iter.first.c_str(), (void *)( iter.second.c_str() ) );
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

struct measurement_id_t {
    int type,subtype;
};

zmsg_t * nutdevice_to_measurement_msg(const NUTDevice &dev, const std::string &name,  int value) {
    static std::map<std::string,measurement_id_t> IDs;
    std::cout << "nut to measurement: " << name << "\n";
    if( IDs.count(name) == 0 ) {
        // we dont have this ID name, lets get it
        std::string typeName = "" , subtypeName = "";
        std::size_t i = name.find(".");
        if( i ) {
            typeName = name.substr(0,i);
            subtypeName = name.substr(i+1);
        }
        common_msg_t * cmsg = common_msg_new(COMMON_MSG_GET_MEASURE_SUBTYPE_SS);
        common_msg_set_mt_name(cmsg,typeName.c_str());
        common_msg_set_mts_name(cmsg,subtypeName.c_str());
        assert(cmsg);
        zmsg_t* reply = process_measures_meta(&cmsg);
        common_msg_destroy(&cmsg);
        if( reply ) {
            cmsg = common_msg_decode(&reply);
            measurement_id_t ID;
            ID.type = common_msg_mt_id(cmsg);
            ID.subtype = common_msg_mts_id(cmsg);
            if( ID.type != 0 && ID.subtype != 0 ) {
                common_msg_print(cmsg);
                IDs[name] = ID;
            }
        }
        //zmsg_destroy(&zmsg);
        zmsg_destroy(&reply);
        if( IDs.count(name) == 0 ) {
            return NULL;
        }
    }
    //if( 
    // FIXME: get nut name from DB
    zmsg_t * zmsg = common_msg_encode_new_measurement(
                                                      "NUT",          
                                                      dev.name().c_str(),
                                                      "ups",
                                                      IDs[name].type,
                                                      IDs[name].subtype,
                                                      value
    );
    return zmsg;
}

void nut_actor(zsock_t *pipe, void *args) {
    log_info ("%s", "nut_actor start\n");

    std::map<std::string,measurement_id_t> measurement_ids;

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
                    //
                    for(auto &measurement : it->second.physics(true) ) {
                        zmsg_t *msg = nutdevice_to_measurement_msg(it->second,measurement.first,measurement.second);
                        if(msg) {
                            log_info ("ups %s : %s\n", it->second.name().c_str(), it->second.toString().c_str() );
                            zmsg_send(&msg,dbsock);
                        }
                        zmsg_destroy(&msg);
                    }
                    //FIXME: don't send allways
                    // zmsg_t *msg = nutdevice_to_powerdev_msg(it->second);
                    // log_info ("ups %s : %s\n", it->second.name().c_str(), it->second.toString().c_str() );
                    // zmsg_send(&msg,dbsock);
                    // zmsg_destroy(&msg);
                    it->second.changed(false);
                }
            }
        }
    }
    zpoller_destroy(&poller);
    zsock_destroy(&dbsock);
    log_info ("%s", "nut_actor end\n");
}

} // namespace drivers::nut
} // namespace drivers

