#include <stdio.h>
#include <iostream>
#include <string>
#include <exception>
#include <errno.h>
#include <tntdb/connect.h>

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
        return true;
    } catch (const std::exception& e) {
        log_error ("Excepton caught: '%s'.", e.what ());
        return false;
    } catch(...) {
        log_error ("Unknown exception caught.");
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
    if( ! configuration() ) {
        log_error("Failed to read configuration from database");
        zsys_interrupted = true;
        _exitStatus = 1;
    }
}

void TotalPowerAgent::onSend( ymsg_t **message ) {
    Measurement M = *message;

    auto affected_it = _affectedRacks.find( M.deviceName() );
    if( affected_it != _affectedRacks.end() ) {
        // this device affects some total rack
        auto rack_it = _racks.find( affected_it->second );
        if( rack_it != _racks.end() ) {
            // affected rack found
            rack_it->second.setMeasurement(M);
            sendMeasurement( _racks );
        }
    }
    affected_it = _affectedDCs.find( M.deviceName() );
    if( affected_it != _affectedDCs.end() ) {
        // this device affects some total DC
        auto dc_it = _DCs.find( affected_it->second );
        if( dc_it != _DCs.end() ) {
            // affected dc found
            dc_it->second.setMeasurement(M);
            sendMeasurement( _DCs );
        }
    }
    _timeout = getPollInterval();
}

void  TotalPowerAgent::sendMeasurement(std::map< std::string, TPUnit > &elements) {
    for( auto &element : elements ) {
        if( element.second.advertise() ) {
            _scoped_ymsg_t *message = element.second.measurementMessage();
            if( message ) {
                std::string topic = "measurement.realpower.default@" + element.second.name();
                Measurement M = element.second.realpower();
                log_debug("Sending total power topic: %s value: %" PRIi32 "*10^%" PRIi32,
                          topic.c_str(),
                          M.value(),
                          M.scale());
                send( topic.c_str(), &message );
                element.second.advertised();
            }
        } else {
            // log something from time to time if device power is unknown
            auto devices = element.second.devicesInUnknownState();
            if( ! devices.empty() ) {
                std::string devicesText;
                for( auto &it: devices ) {
                    devicesText += it + " ";
                }
                log_error("Devices preventing total power calculation for %s are: %s", element.first.c_str(), devicesText.c_str() );
            }
        }
    }
}

time_t TotalPowerAgent::getPollInterval() {
    time_t T = TPOWER_MEASUREMENT_REPEAT_AFTER;
    for( auto &rack_it : _racks ) {
        time_t Tx = rack_it.second.timeToAdvertisement();
        if( Tx > 0 && Tx < T ) T = Tx;
    }
    for( auto &dc_it : _racks ) {
        time_t Tx = dc_it.second.timeToAdvertisement();
        if( Tx > 0 && Tx < T ) T = Tx;
    }
    return T * 1000;
}


void TotalPowerAgent::onPoll() {
    sendMeasurement( _racks );
    sendMeasurement( _DCs );
    _timeout = getPollInterval();
}

int main(int argc, char *argv[]){
    if( argc > 0 ) {}; // silence compiler warnings
    if( argv ) {};  // silence compiler warnings
    
    int result = 1;
    log_open();
    log_set_level(LOG_DEBUG);
    log_info ("tpower agent started");
    TotalPowerAgent agent("TPOWER");
    if( agent.connect(MLM_ENDPOINT, bios_get_stream_main(), "^measurement\\.realpower\\.default@") ) {
        result = agent.run();
    }
    log_info ("tpower agent exited with code %i\n", result);
    return result;
}
