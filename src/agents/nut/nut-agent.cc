#include <stdio.h>

#include "nut-agent.h"
#include "ymsg.h"
#include "log.h"
#include "agents.h"
#include "upsstatus.h"
#include "utils_ymsg.h"

// TODO: read this from configuration (once in 5 minutes now (300s))
#define NUT_MEASUREMENT_REPEAT_AFTER 300
// TODO: read this from configuration (every hour now (3600s))
#define NUT_INVENTORY_REPEAT_AFTER 3600
// TODO: read this from configuration (check with upsd ever 5s)
#define NUT_POLLING_INTERVAL  5000

void NUTAgent::onStart( ) {
    _timeout = NUT_POLLING_INTERVAL;
}


void NUTAgent::onPoll() {
    advertisePhysics();
    advertiseInventory();
}

std::string NUTAgent::physicalQuantityShortName(const std::string &longName) {
    size_t i = longName.find('.');
    if( i == std::string::npos) return longName;
    return longName.substr(0, i);
}

std::string NUTAgent::physicalQuantityToUnits(const std::string &quantity) {
    auto it = _units.find(quantity);
    if( it == _units.end() ) return "";
    return it->second;
}

void NUTAgent::advertisePhysics() {
    bool advertise = false;
    if( _measurementTimestamp + NUT_MEASUREMENT_REPEAT_AFTER < std::time(NULL) ) {
        advertise = true;
        _measurementTimestamp = std::time(NULL);
    }
    _deviceList.update( advertise );
    for( auto &device : _deviceList ) {
        std::string topic;
        if( advertise || device.second.changed() ) {
            for(auto &measurement : device.second.physics( ! advertise ) ) {
                topic = "measurement." + measurement.first + "@" + device.second.name();
                std::string type = physicalQuantityShortName(measurement.first);
                std::string units = physicalQuantityToUnits(type);
                if( units.empty() ) {
                    log_error("undefined physical quantity %s",type.c_str() );
                    continue;
                }
                // Create message and publish it
                ymsg_t *msg = bios_measurement_encode(
                    device.second.name().c_str(),
                    measurement.first.c_str(),
                    units.c_str(),
                    measurement.second, -2, -1);
                if( msg ) {
                    log_debug("sending new measurement for ups %s, type %s, value %" PRIi32,
                              device.second.name().c_str(),
                              measurement.first.c_str(),
                              measurement.second );
                    send( topic.c_str(), &msg );
                    ymsg_destroy(&msg);
                    device.second.setChanged(measurement.first,false);
                }
            }
            // send also status as bitmap
            if( device.second.hasProperty("status") && ( advertise || device.second.changed("status") ) ) {
                topic = "measurement.status@" + device.second.name();
                std::string status_s = device.second.property("status");
                uint16_t    status_i = shared::upsstatus_to_int( status_s );
                ymsg_t *msg = bios_measurement_encode(
                    device.second.name().c_str(),
                    "status",
                    "",
                    status_i, 0, -1);
                if( msg ) {
                    log_debug("sending new status for ups %s, value %i (%s)", device.second.name().c_str(), status_i, status_s.c_str() );
                    send( topic.c_str(), &msg );
                    ymsg_destroy(&msg);
                    device.second.setChanged("status",false);
                }
            }
        }
    }
}

void NUTAgent::advertiseInventory() {
    bool advertise = false;
    if( _inventoryTimestamp + NUT_INVENTORY_REPEAT_AFTER < std::time(NULL) ) {
        advertise = true;
        _inventoryTimestamp = std::time(NULL);
    }
    for( auto &device : _deviceList ) {
        std::string topic = "inventory@" + device.second.name();
        zhash_t *inventory = zhash_new();
        for( auto &item : device.second.inventory( !advertise ) ) {
            if( item.first != "status" ) { 
                zhash_insert( inventory, item.first.c_str(), (void *)item.second.c_str() );
                device.second.setChanged(item.first,false);
            }
        }
        if( zhash_size(inventory) > 0 ) {
            ymsg_t *message = bios_inventory_encode(
                device.second.name().c_str(),
                &inventory,
                "inventory" );
            if( message ) {
                bios_agent_send( _bios_agent, topic.c_str(), &message );
                ymsg_destroy( &message );
            }
        }
        zhash_destroy( &inventory );
    }
}

int main(int argc, char *argv[]){
    if( argc > 0 ) {}; // silence compiler warnings
    if( argv ) {};  // silence compiler warnings
    
    int result = 1;
    log_open();
    log_set_level(LOG_DEBUG);
    log_info ("nut agent started");
    NUTAgent agent("NUT");
    if( agent.connect("ipc://@/malamute","bios",NULL) ) {
        result = agent.run();
    }
    log_info ("nut agent exited with code %i\n", result);
    return result;
}
