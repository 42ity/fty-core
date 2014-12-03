#include <string>
#include <ctime>
#include "nut-driver.h"
#include "powerdev_msg.h"
#include "measure_types.h"
#include "common_msg.h"
#include "defs.h"
#include "log.h"
#include "upsstatus.h"

namespace drivers
{
namespace nut
{

// TODO: read this from configuration (once in 5 minutes now (300s))
#define NUT_MESSAGE_REPEAT_AFTER 300
// TODO: read this from configuration (check with upsd ever 5s)
#define NUT_POLLING_INTERVAL  5000

/**
 * \brief private sructure for caching measurement IDs
 */
struct measurement_id_t {
    uint16_t type, subtype;
    signed char scale;
};

// basic powerdev properties
static const std::vector<std::string> basicProperties {
    "model",
    "manufacturer",
    "serial",
    "type",
    "status",
};

zmsg_t *nut_device_to_powerdev_msg(const NUTDevice &dev) {
    zmsg_t *msg;
    char *name[basicProperties.size()];

    zhash_t *otherProperties = zhash_new();
    assert(otherProperties);
    auto properties = dev.properties();

    memset(name, 0, sizeof(char *) * basicProperties.size() );
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

/**
 * \brief scalling internal nut values to database scale
 */
long int nut_scale(long int value, int scale ) {
    // NUT driver uses -2 scale internally
    scale += 2;
    if( scale == 0 ) { return value; }
    if( scale > 0 ) {
        return value / pow(10, scale);
    }
    return value * pow(10, -scale);
}

/**
 * \brief function for discovering measurement IDs
 */
measurement_id_t nut_get_measurement_id(const std::string &name) {
    common_msg_t *cmsg;
    zmsg_t *request, *reply;
    measurement_id_t ID;
    static std::map<std::string, std::string> units = {
        { "temperature", "C" },
        { "realpower",   "W" },
        { "voltage",     "V" },
        { "current",     "A" },
        { "load",        "%" },
        { "charge",      "%" },
    };
    static std::map<std::string, int8_t> scales = {
        { "status.ups", 0 },
    };

    memset(&ID, 0, sizeof(ID));
    std::string typeName = "" , subtypeName = "";
    std::size_t i = name.find(".");
    if( i ) {
        typeName = name.substr(0, i);
        subtypeName = name.substr(i+1);

        // Get type info
        auto unit = units.find(typeName);
        request = common_msg_encode_get_measure_type_s(
                        typeName.c_str(),
                        ((unit == units.end()) ?
                            std::string("") :
                            unit->second).c_str());
        reply = process_measures_meta(&request);
        zmsg_destroy(&request);
        if( reply ) {
            cmsg = common_msg_decode(&reply);
            assert(common_msg_mt_unit(cmsg) ==
                   ((unit == units.end()) ? "" : unit->second));
            ID.type = common_msg_mt_id(cmsg);
            common_msg_destroy(&cmsg);
        }
        zmsg_destroy(&reply);

        // Get subtype info
        auto scale = scales.find(name);
        request = common_msg_encode_get_measure_subtype_s(
                        ID.type,
                        subtypeName.c_str(),
                        (uint8_t)((scale == scales.end()) ?
                                   -2 : scale->second));
        reply = process_measures_meta(&request);
        zmsg_destroy(&request);
        if( reply ) {
            cmsg = common_msg_decode(&reply);
            ID.subtype = common_msg_mts_id(cmsg);
            ID.scale = (int8_t)common_msg_mts_scale(cmsg);
            assert(ID.scale == ((scale == scales.end()) ?
                                 -2 : scale->second));
            common_msg_destroy(&cmsg);
        }
        zmsg_destroy(&reply);
    } else {
        log_error("NUT invalid measurement id name found, %s doesn't contain subtype\n", name.c_str());
    }
    return ID;
}

/**       
 * \brief creating powerdev message from NUTDevice 
 */
zmsg_t * nut_device_to_measurement_msg(const NUTDevice &dev, const std::string &name,  int value) {
    static std::map<std::string, measurement_id_t> IDs;
    zmsg_t *zmsg;
    measurement_id_t ID;
   
    if( IDs.count(name) == 0 ) {
        // we dont know measurement IDs
        ID = nut_get_measurement_id(name);
        if( ID.type != 0 && ID.subtype != 0 ) {
            IDs[name] = ID;
            log_debug("Measurement type and subtype for %s is %i/%i scale %i\n", name.c_str(), ID.type, ID.subtype, ID.scale );
        } else {
            log_error("Can't get measurement type and subtype for %s\n", name.c_str());
            return NULL;
        }
    }
    if( dev.hasProperty("type") ) {
        std::string type = dev.property("type");
        // TODO: get NUT name from DB/is it necessary?
        zmsg = common_msg_encode_new_measurement(
            "NUT",          
            dev.name().c_str(),
            type.c_str(),
            IDs[name].type,
            IDs[name].subtype,
            nut_scale(value, IDs[name].scale)
        );
        return zmsg;
    } else {
        log_error("NUT device %s type is unknown\n", dev.name().c_str()); 
        return NULL;
    }
}

// we know that *args param in nut_actor() is unused parameter
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * \brief main nut-driver loop. Checks NUT every 5 second for changes.
 *        detected changes are propagated immediatelly. Once in
 *        NUT_MESSAGE_REPEAT_AFTER interval [seconds], all values are
 *        propagated, even if there is nochange.
 */
void nut_actor(zsock_t *pipe, void *args) {
    log_info ("%s", "nut_actor start\n");

    bool advertise;
    std::map<std::string, measurement_id_t> measurement_ids;
    std::time_t timestamp = std::time(NULL);

    zsock_t * dbsock = zsock_new_dealer(DB_SOCK);
    assert(dbsock);
    zpoller_t *poller = zpoller_new(pipe, NULL);
    assert(poller);
    zsock_signal(pipe, 0);
    NUTDeviceList listOfUPS;

    while(!zpoller_terminated(poller)) {
        zsock_t *which = (zsock_t *)zpoller_wait(poller, NUT_POLLING_INTERVAL);
        if (which == pipe) {
            break;
        } else if( which == NULL ) {
            if( timestamp + NUT_MESSAGE_REPEAT_AFTER < time(NULL) ) {
                // timestamp + NUT_MESSAGE_REPEAT_AFTER is in past
                advertise = true;
                timestamp = std::time(NULL);
            } else {
                advertise = false;
            }
            listOfUPS.update();
            for(auto it = listOfUPS.begin(); it != listOfUPS.end() ; ++it) {
                if(advertise || it->second.changed() ) {
                    // something has changed or we should advertise status
                    // go trough measurements
                    for(auto &measurement : it->second.physics( ! advertise ) ) {
                        zmsg_t *msg = nut_device_to_measurement_msg(it->second, measurement.first, measurement.second);
                        if(msg) {
                            log_debug("sending new measurement for ups %s, type %s, value %i\n", it->second.name().c_str(), measurement.first.c_str(),measurement.second ); 
                            zmsg_send(&msg, dbsock);
                        }
                        zmsg_destroy(&msg);
                    }
                    // send also status as bitmap
                    if( it->second.hasProperty("status") && ( advertise || it->second.changed("status") ) ) {
                        std::string status_s = it->second.property("status");
                        uint16_t    status_i = shared::upsstatus_to_int( status_s );
                        zmsg_t *msg = nut_device_to_measurement_msg(it->second, "status.ups", status_i);
                        if(msg) {
                            log_debug("sending new status for ups %s, value %i (%s)\n", it->second.name().c_str(), status_i, status_s.c_str() );
                            zmsg_send(&msg, dbsock);
                        }
                        zmsg_destroy(&msg);
                    }
                    // send also POWERDEV_MSG_POWERDEV_STATUS
                    zmsg_t *msg = nut_device_to_powerdev_msg(it->second);
                    log_info ("ups %s snapshot: %s\n", it->second.name().c_str(), it->second.toString().c_str() );
                    zmsg_send(&msg, dbsock);
                    zmsg_destroy(&msg);
                    it->second.setChanged(false);
                }
            }
        }
    }
    zpoller_destroy(&poller);
    zsock_destroy(&dbsock);
    log_info ("%s", "nut_actor end\n");
}

#pragma GCC diagnostic pop


} // namespace drivers::nut
} // namespace drivers
