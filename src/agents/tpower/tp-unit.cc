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

Measurement TPUnit::summarize(const std::string &quantity) const{
    Measurement result;
    result.units("W");
    for( auto &it : _powerdevices ) {
        auto itMeasurements = getMeasurementIter( it.second, quantity );
        if( itMeasurements != it.second.end() ) {
            result += itMeasurements->second;
        }
    }
    return result;
}

std::map<std::string,Measurement>::const_iterator TPUnit::getMeasurementIter(
    const std::map<std::string,Measurement> &measurements,
    const std::string &quantity
) const
{
    const auto result = measurements.find(quantity);
    if( result != measurements.end() ) {
        // we have it
        return result;
    }
    const auto &replacement = _emergencyReplacements.find(quantity);
    if( replacement == _emergencyReplacements.end() ) {
        // there is no replacement for this value, return measurements.end()
        return result;
    }
    // find the replacement value if any
    log_debug("%s is unknown, trying %s instead", quantity.c_str(), replacement->second.c_str() );
    const auto result_replace = measurements.find( replacement->second );
    if( result_replace == measurements.end() ) {
        log_debug("replacement value of %s is unknown too", replacement->second.c_str() );
    } else {
        log_debug("replacement value of %s found", replacement->second.c_str() );
    }
    return result_replace;
}

bool TPUnit::quantityIsUnknown(const std::string &quantity) const
{
    time_t now = std::time(NULL);
    for( const auto &it : _powerdevices ) {
        const auto itMeasurements = getMeasurementIter( it.second, quantity );
        if( itMeasurements == it.second.end() ) return true;
        if( now - itMeasurements->second.time() > TPOWER_MEASUREMENT_REPEAT_AFTER * 2 ) return true;
    }
    return false;
}

std::vector<std::string> TPUnit::devicesInUnknownState(const std::string &quantity) const
{
    std::vector<std::string> result;
    time_t now = std::time(NULL);
    for( auto &it : _powerdevices ) {
        auto itMeasurements = getMeasurementIter( it.second, quantity );
        if(
            itMeasurements == it.second.end()  ||
            ( now - itMeasurements->second.time() > TPOWER_MEASUREMENT_REPEAT_AFTER * 2 )
        ) result.push_back( it.first );
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
        if( _powerdevices[M.deviceName()][M.source()] != M ) changed( M.source(), true );
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
        Measurement M = summarize(quantity);
        ymsg_t *message = bios_measurement_encode(
            _name.c_str(),
            quantity.c_str(),
            M.units().c_str(),
            M.value(),
            M.scale(),
            0
        );
        return message;
    } catch(...) {
        return NULL;
    }    
}
