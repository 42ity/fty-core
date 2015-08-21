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

Measurement TPUnit::realpower() const{
    Measurement result;
    result.units("W");
    for( auto &it : _powerdevices ) {
        result += it.second;
    }
    return result;
}

bool TPUnit::realpowerIsUnknown() const
{
    time_t now = std::time(NULL);
    for( auto &it : _powerdevices ) {
        if( now - it.second.time() > TPOWER_MEASUREMENT_REPEAT_AFTER * 2 ) return true;
    }
    return false;
}

std::vector<std::string> TPUnit::devicesInUnknownState() const
{
    std::vector<std::string> result;
    time_t now = std::time(NULL);
    for( auto &it : _powerdevices ) {
        if( now - it.second.time() > TPOWER_MEASUREMENT_REPEAT_AFTER * 2 ) result.push_back( it.first );
    }
    return result;
}

void TPUnit::addPowerDevice(const std::string &device)
{
    _powerdevices[device] = Measurement();
}

void TPUnit::setMeasurement(const Measurement &M)
{
    if( _powerdevices.find( M.deviceName() ) != _powerdevices.end() ) {
        if( _powerdevices[M.deviceName()] != M ) changed(true);
        _powerdevices[M.deviceName()] = M;
    }
}

void TPUnit::changed(bool newStatus) {
    if( _changed != newStatus ) {
        _changed = newStatus;
        _timestamp = time(NULL);
    }
}

void TPUnit::advertised() {
    _changed = false;
    _timestamp = time(NULL);
}

time_t TPUnit::timeToAdvertisement() const {
    if( ( _timestamp == 0 ) || realpowerIsUnknown() ) return TPOWER_MEASUREMENT_REPEAT_AFTER;
    time_t dt = time(NULL) - _timestamp;
    if( dt > TPOWER_MEASUREMENT_REPEAT_AFTER ) return 0;
    return TPOWER_MEASUREMENT_REPEAT_AFTER - dt;
}

bool TPUnit::advertise() const{
    if( realpowerIsUnknown() ) return false;
    return ( _changed || ( time(NULL) - _timestamp > TPOWER_MEASUREMENT_REPEAT_AFTER ) );
}

ymsg_t * TPUnit::measurementMessage() {
    try {
        Measurement M = realpower();
        ymsg_t *message = bios_measurement_encode(
            _name.c_str(),
            "realpower.default",
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
