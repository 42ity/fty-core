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
 * \file agent-tpower.cc
 * \author Tomas Halman <TomasHalman@Eaton.com>
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \author Jim Klimov <EvgenyKlimov@Eaton.com>
 * \brief Not yet documented file
 */
#include <stdio.h>
#include <iostream>
#include <string>
#include <exception>
#include <errno.h>
#include <tntdb/connect.h>
#include <preproc.h>

#include "agent-tpower.h"
#include "str_defs.h"
#include "log.h"
#include "cleanup.h"
#include "dbhelpers.h"
#include "calc_power.h"
#include "dbpath.h"

bool TotalPowerAgent::configuration( )
{
    tntdb::Connection connection;

    try {
        log_info("loading power topology from database");
        _racks.clear();
        _affectedRacks.clear();
        _DCs.clear();
        _affectedDCs.clear();
        
        connection = tntdb::connect(url);
        // reading racks
        db_reply <std::map<std::string, std::vector<std::string>>> ret =
            select_devices_total_power_racks (connection);

        if( ret.status ) {
            for( auto & rack_it: ret.item ) {
                log_debug("reading rack %s powerdevices", rack_it.first.c_str() );
                auto devices = rack_it.second;
                for( auto device_it: devices ) {
                    log_debug("rack %s powerdevice %s", rack_it.first.c_str(), device_it.c_str() );
                    addDeviceToMap(_racks, _affectedRacks, rack_it.first, device_it );
                }
            }
        }
        // reading DCs
        ret = select_devices_total_power_dcs (connection);
        if( ret.status ) {
            for( auto & dc_it: ret.item ) {
                log_debug("reading DC %s powerdevices", dc_it.first.c_str() );
                auto devices = dc_it.second;
                for( auto device_it: devices ) {
                    log_debug("DC %s powerdevice %s", dc_it.first.c_str(), device_it.c_str() );
                    addDeviceToMap(_DCs, _affectedDCs, dc_it.first, device_it );
                }
            }
        }
        connection.close();
        _reconfigPending = 0;
        return true;
    } catch (const std::exception& e) {
        log_error("Failed to read configuration from database. Excepton caught: '%s'.", e.what ());
        _reconfigPending = time(NULL) + 60;
        return false;
    } catch(...) {
        log_error ("Failed to read configuration from database. Unknown exception caught.");
        _reconfigPending = time(NULL) + 60;
        return false;
    }
}

void TotalPowerAgent::addDeviceToMap(
    std::map< std::string, TPUnit > &elements,
    std::map< std::string, std::string > &reverseMap,
    const std::string & owner,
    const std::string & device )
{
    auto element = elements.find(owner);
    if( element == elements.end() ) {
        auto box = TPUnit();
        box.name(owner);
        box.addPowerDevice(device);
        elements[owner] = box;
    } else {
        element->second.addPowerDevice(device);
    }
    reverseMap[device] = owner;
}

void TotalPowerAgent::onStart( )
{
    _timeout = TPOWER_POLLING_INTERVAL;
    configuration();
}

void TotalPowerAgent::onSend( ymsg_t **message ) {
    std::string topic = subject();
    log_debug("received message with topic \"%s\"", topic.c_str() );
    if( topic.compare(0,9,"configure") == 0 ) {
        // something is beeing reconfigured, let things to settle down
        if( _reconfigPending == 0 ) log_info("Reconfiguration scheduled");
        _reconfigPending = time(NULL) + 60;
    } else {
        // measurement received
        Measurement M = *message;

        if( _rackRegex.match(topic) ) {
            auto affected_it = _affectedRacks.find( M.deviceName() );
            if( affected_it != _affectedRacks.end() ) {
                // this device affects some total rack
                log_debug("measurement is interesting for rack %s", affected_it->second.c_str() );
                auto rack_it = _racks.find( affected_it->second );
                if( rack_it != _racks.end() ) {
                    // affected rack found
                    rack_it->second.setMeasurement(M);
                    sendMeasurement( _racks, _rackQuantities );
                }
            }
        }
        if( _dcRegex.match(topic) ) {
            auto affected_it = _affectedDCs.find( M.deviceName() );
            if( affected_it != _affectedDCs.end() ) {
                // this device affects some total DC
                log_debug("measurement is interesting for DC %s", affected_it->second.c_str() );
                auto dc_it = _DCs.find( affected_it->second );
                if( dc_it != _DCs.end() ) {
                    // affected dc found
                    dc_it->second.setMeasurement(M);
                    sendMeasurement( _DCs, _dcQuantities );
                }
            }
        }
    }
    _timeout = getPollInterval();
}

void  TotalPowerAgent::sendMeasurement(std::map< std::string, TPUnit > &elements, const std::vector<std::string> &quantities ) {
    for( auto &element : elements ) {
        for( auto &q : quantities ) {
            if( element.second.advertise(q) ) {
                _scoped_ymsg_t *message = element.second.measurementMessage(q);
                if( message ) {
                    std::string topic = "measurement." + q + "@" + element.second.name();
                    Measurement M = element.second.summarize(q);
                    log_debug("Sending total power topic: %s value: %" PRIi32 "*10^%" PRIi32,
                              topic.c_str(),
                              M.value(),
                              M.scale());
                    send( topic.c_str(), &message );
                    element.second.advertised(q);
                }
            } else {
                // log something from time to time if device calculation is unknown
                auto devices = element.second.devicesInUnknownState(q);
                if( ! devices.empty() ) {
                    std::string devicesText;
                    for( auto &it: devices ) {
                        devicesText += it + " ";
                    }
                    log_info("Devices preventing total %s calculation for %s are: %s",
                             q.c_str(),
                             element.first.c_str(),
                             devicesText.c_str() );
                }
            }
        }
    }
}

time_t TotalPowerAgent::getPollInterval() {
    time_t T = TPOWER_MEASUREMENT_REPEAT_AFTER;
    for( auto &rack_it : _racks ) {
        for( auto &q : _rackQuantities ) {
            time_t Tx = rack_it.second.timeToAdvertisement(q);
            if( Tx > 0 && Tx < T ) T = Tx;
        }
    }
    for( auto &dc_it : _racks ) {
        for( auto &q : _dcQuantities ) {
            time_t Tx = dc_it.second.timeToAdvertisement(q);
            if( Tx > 0 && Tx < T ) T = Tx;
        }
    }
    if( _reconfigPending ) {
        time_t Tx = _reconfigPending - time(NULL) + 1;
        if( Tx <= 0 ) Tx = 1;
        if( Tx < T ) T = Tx;
    }
    return T * 1000;
}


void TotalPowerAgent::onPoll() {
    sendMeasurement( _racks, _rackQuantities );
    sendMeasurement( _DCs, _dcQuantities );
    if( _reconfigPending && ( _reconfigPending <= time(NULL) ) ) configuration();
    _timeout = getPollInterval();
}

int main( UNUSED_PARAM int argc, UNUSED_PARAM char *argv[] ) {
    int result = 1;
    
    log_open();
    log_info ("tpower agent started");
    TotalPowerAgent agent("TPOWER");
    if( agent.connect(MLM_ENDPOINT, bios_get_stream_main(), "^(measurement\\.realpower\\..+@|configure@)") ) {
        result = agent.run();
    }
    log_info ("tpower agent exited with code %i\n", result);
    return result;
}
