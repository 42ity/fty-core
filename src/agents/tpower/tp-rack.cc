#include "tp-rack.h"
#include "agent-tpower.h"
#include "agents.h"

#include <ctime>

Measurement TPRack::realpower() {
    Measurement result;
    result.units("W");
    for( auto &it : _powerdevices ) {
        result += it.second;
    }
    return result;
}

bool TPRack::realpowerIsUnknown() const
{
    time_t now = std::time(NULL);
    for( auto &it : _powerdevices ) {
        if( now - it.second.time() > TPOWER_MEASUREMENT_REPEAT_AFTER*2 ) return true;
    }
    return false;
}

void TPRack::addPowerDevice(const std::string &device)
{
    _powerdevices[device] = Measurement();
}

void TPRack::setMeasurement(const Measurement &M)
{
    if( _powerdevices.find( M.deviceName() ) != _powerdevices.end() ) {
        if( _powerdevices[M.deviceName()] != M ) {
            _powerdevices[M.deviceName()] = M;
            changed(true);
        }
    }
}

void TPRack::changed(bool newStatus) {
    if( _changed != newStatus ) {
        _changed = newStatus;
        _timestamp = time(NULL);
    }
}

void TPRack::advertised() {
    _changed = false;
    _timestamp = time(NULL);
}

bool TPRack::advertise() {
    if( realpowerIsUnknown() ) return false;
    // FIXME remove  _AFTER/5
    return ( _changed || ( time(NULL) - _timestamp > (TPOWER_MEASUREMENT_REPEAT_AFTER/5) ) );
}

ymsg_t * TPRack::measurementMessage() {
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
