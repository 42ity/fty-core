/*
 *
 * Copyright (C) 2015 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*!
 * \file agent-nut.cc
 * \author Tomas Halman <TomasHalman@Eaton.com>
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \author Jim Klimov <EvgenyKlimov@Eaton.com>
 * \brief Not yet documented file
 */
#include <stdio.h>
#include <ctime>

#include "agent-nut.h"
#include "ymsg.h"
#include "log.h"
#include "agents.h"
#include "upsstatus.h"
#include "utils_ymsg.h"
#include "cleanup.h"
#include "defs.h"

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
                _scoped_ymsg_t *msg = bios_measurement_encode(
                    device.second.name().c_str(),
                    measurement.first.c_str(),
                    units.c_str(),
                    measurement.second, -2, std::time(0));
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
            if( device.second.hasProperty("status.ups") && ( advertise || device.second.changed("status.ups") ) ) {
                topic = "measurement.status@" + device.second.name();
                std::string status_s = device.second.property("status.ups");
                uint16_t    status_i = shared::upsstatus_to_int( status_s );
                _scoped_ymsg_t *msg = bios_measurement_encode(
                    device.second.name().c_str(),
                    "status.ups",
                    "",
                    status_i, 0, std::time(0));
                if( msg ) {
                    log_debug("sending new status for ups %s, value %i (%s)", device.second.name().c_str(), status_i, status_s.c_str() );
                    send( topic.c_str(), &msg );
                    ymsg_destroy(&msg);
                    device.second.setChanged("status.ups",false);
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
        std::string log;
        zhash_t *inventory = zhash_new();
        for( auto &item : device.second.inventory( !advertise ) ) {
            if( item.first != "status.ups" ) { 
                zhash_insert( inventory, item.first.c_str(), (void *)item.second.c_str() );
                log += item.first + " = \"" + item.second + "\"; ";
                device.second.setChanged(item.first,false);
            }
        }
        if( zhash_size(inventory) > 0 ) {
            _scoped_ymsg_t *message = bios_inventory_encode(
                device.second.name().c_str(),
                &inventory,
                "inventory" );
            if( message ) {
                log_debug( "new inventory message %s: %s", topic.c_str(), log.c_str() );
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
    log_info ("nut agent started");
    NUTAgent agent("NUT");
    if( agent.connect("ipc://@/malamute", bios_get_stream_main(), NULL) ) {
        result = agent.run();
    }
    log_info ("nut agent exited with code %i\n", result);
    return result;
}
