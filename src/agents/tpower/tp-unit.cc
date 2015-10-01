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
 * \file tp-unit.cc
 * \author Tomas Halman <TomasHalman@Eaton.com>
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \brief Not yet documented file
 */
#include <ctime>
#include <exception>

#include "tp-unit.h"
#include "agent-tpower.h"
#include "agents.h"
#include "log.h"

/**
 * \brief mapping table for missing values replacements
 *
 * This table says simply "if we don't have measurement for realpower.input.L1, we can
 * use realpower.output.L1 instead".
 */
const std::map<std::string,std::string> TPUnit::_emergencyReplacements = {
    { "realpower.input.L1", "realpower.output.L1" },
    { "realpower.input.L2", "realpower.output.L2" },
    { "realpower.input.L3", "realpower.output.L3" },
};

enum TPowerMethod {
    tpower_realpower_default = 1,
};

const std::map<std::string,int> TPUnit::_calculations = {
    { "realpower.default", tpower_realpower_default },
};

std::map<std::string,Measurement>::const_iterator TPUnit::get( const std::string &quantity) const {
    if( quantityIsUnknown( quantity ) ) return _lastValue.cend();
    return _lastValue.find(quantity);
}

void TPUnit::set(const std::string &quantity, Measurement measurement)
{
    auto itSums = _lastValue.find(quantity);
    if( itSums == _lastValue.end() || (itSums->second != measurement) ) {
        _lastValue[quantity] = measurement;
        _changed[quantity] = true;
        _changetimestamp[quantity] = time(NULL);
    }
    _updatetimestamp[quantity] = time(NULL);
}

Measurement TPUnit::simpleSummarize(const std::string &quantity) const {
    Measurement result;
    result.units("W");
    for( const auto it : _powerdevices ) {
        auto itMeasurements = getMeasurementIter( it.second, quantity, it.first );
        if( itMeasurements == it.second.end() ) {
            throw std::runtime_error("value can't be calculated");
        } else {
            result += itMeasurements->second;
        }
    }
    return result;
}

Measurement TPUnit::realpowerDefault(const std::string &quantity) const {
    Measurement result;
    result.units("W");
    for( const auto it : _powerdevices ) {
        auto itMeasurements = getMeasurementIter( it.second, quantity, it.first );
        if( itMeasurements == it.second.end() ) {
            // realpower.default not present, try to sum the phases
            for( int phase = 1 ; phase <= 3 ; ++phase ) {
                auto itItem = getMeasurementIter( it.second, "realpower.input.L" + std::to_string( phase ), it.first );
                if( itItem == it.second.end() ) {
                    throw std::runtime_error("value can't be calculated");
                }
                result += itItem->second;
            }       
        } else {
            result += itMeasurements->second;
        }
    }
    return result;
}

void TPUnit::calculate(const std::vector<std::string> &quantities) {
    dropOldMeasurements();
    for( const auto it : quantities ) {
        calculate( it );
    }
}

void TPUnit::calculate(const std::string &quantity) {
    try {
        Measurement result;
        int calc = 0;
        const auto how = _calculations.find(quantity);
        if( how != _calculations.cend() ) calc = how->second;
        switch( calc ) {
        case tpower_realpower_default:
            result = realpowerDefault( quantity );
            break;
        default:
            result = simpleSummarize( quantity );
            break;
        }
        set( quantity, result );
    } catch (...) { }
}

std::map<std::string,Measurement>::const_iterator TPUnit::getMeasurementIter(
    const std::map<std::string,Measurement> &measurements,
    const std::string &quantity,
    const std::string &deviceName
) const
{
    const auto result = measurements.find(quantity);
    if( result != measurements.end() ) {
        // we have it
        return result;
    }
    const auto &replacement = _emergencyReplacements.find(quantity);
    if( replacement == _emergencyReplacements.cend() ) {
        // there is no replacement for this value, return measurements.end()
        return result;
    }
    // find the replacement value if any
    const auto result_replace = measurements.find( replacement->second );
    if( result_replace == measurements.end() ) {
        log_info("device %s, value of %s is unknown",
                 deviceName.c_str(),
                 replacement->second.c_str() );
    } else {
        log_debug("device %s, using replacement value %s instead of %s",
                  deviceName.c_str(),
                  replacement->second.c_str(),
                  quantity.c_str() );
    }
    return result_replace;
}

void TPUnit::dropOldMeasurements()
{
    time_t now = std::time(NULL);
    for( auto device : _powerdevices ) {
        auto measurement = device.second.begin();
        while( measurement != device.second.end() ) {
            if ( now - measurement->second.time() > TPOWER_MEASUREMENT_REPEAT_AFTER * 2 ) {
                measurement = device.second.erase(measurement);
            } else {
                ++measurement;
            }
        }
    }
}

bool TPUnit::quantityIsUnknown(const std::string &quantity) const
{
    if( _lastValue.find(quantity) == _lastValue.cend() ) {
        return true;
    }
    auto const ts = _updatetimestamp.find( quantity );
    if( ts != _updatetimestamp.cend() && std::time(NULL) - ts->second > TPOWER_MEASUREMENT_REPEAT_AFTER * 2 ) {
        return true;
    }
    return false;
}

std::vector<std::string> TPUnit::devicesInUnknownState(const std::string &quantity) const
{
    std::vector<std::string> result;

    if( quantityIsKnown( quantity ) ) return result;

    time_t now = std::time(NULL);
    for( auto device : _powerdevices ) {
        auto measurement = getMeasurementIter( device.second, quantity, device.first );
        if(
            measurement == device.second.end()  ||
            ( now - measurement->second.time() > TPOWER_MEASUREMENT_REPEAT_AFTER * 2 )
        ) result.push_back( device.first );
    }
    return result;
}

void TPUnit::addPowerDevice(const std::string &device)
{
    _powerdevices[device] = {};
}

void TPUnit::setMeasurement(const Measurement &M)
{
    if( _powerdevices.find( M.deviceName() ) != _powerdevices.end() ) {
        _powerdevices[M.deviceName()][M.source()] = M;
    }
}

bool TPUnit::changed(const std::string &quantity) const {
    auto it = _changed.find(quantity);
    if( it == _changed.end() ) return false;
    return it->second;
}

void TPUnit::changed(const std::string &quantity, bool newStatus) {
    if( changed( quantity ) != newStatus ) {
        _changed[quantity] = newStatus;
        _changetimestamp[quantity] = time(NULL);
        if( _advertisedtimestamp.find(quantity) == _advertisedtimestamp.end() ) {
            _advertisedtimestamp[quantity] = 0;
        }
    }
}

void TPUnit::advertised(const std::string &quantity) {
    changed( quantity, false );
    _changetimestamp[quantity] = time(NULL);
    _advertisedtimestamp[quantity] = time(NULL);
}

time_t TPUnit::timestamp( const std::string &quantity ) const {
    auto it = _changetimestamp.find(quantity);
    if( it == _changetimestamp.end() ) return 0;
    return it->second;
}

time_t TPUnit::timeToAdvertisement( const std::string &quantity ) const {
    if(
        ( timestamp(quantity) == 0 ) ||
        quantityIsUnknown(quantity)
    ) return TPOWER_MEASUREMENT_REPEAT_AFTER;
    time_t dt = time(NULL) - timestamp( quantity );
    if( dt > TPOWER_MEASUREMENT_REPEAT_AFTER ) return 0;
    return TPOWER_MEASUREMENT_REPEAT_AFTER - dt;
}

bool TPUnit::advertise( const std::string &quantity ) const{
    if( quantityIsUnknown(quantity) ) return false;
    const auto it = _advertisedtimestamp.find(quantity);
    if( ( it != _advertisedtimestamp.end() ) && ( it->second == time(NULL) ) ) return false;
    return ( changed(quantity) || ( time(NULL) - timestamp(quantity) > TPOWER_MEASUREMENT_REPEAT_AFTER ) );
}

ymsg_t * TPUnit::measurementMessage( const std::string &quantity ) {
    try {
        const auto M = get(quantity);
        if( M != _lastValue.cend() ) {
                ymsg_t *message = bios_measurement_encode(
                    _name.c_str(),
                    quantity.c_str(),
                    M->second.units().c_str(),
                    M->second.value(),
                    M->second.scale(),
                    0
                    );
                return message;
        }
    } catch(...) {
    }    
    return NULL;
}
