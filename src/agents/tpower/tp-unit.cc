#include <ctime>

#include "tp-unit.h"
#include "agent-tpower.h"
#include "agents.h"
#include "log.h"

Measurement TPUnit::realpower() {
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

bool TPUnit::advertise() {
    if( realpowerIsUnknown() ) return false;
    return ( _changed || ( time(NULL) - _timestamp > TPOWER_MEASUREMENT_REPEAT_AFTER ) );
}

ymsg_t * TPUnit::measurementMessage() {
    ymsg_t *message = NULL;
    try {
        Measurement M = realpower();
        message = bios_measurement_encode(
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
