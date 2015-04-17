#include "alert-measurement.h"

#include<iostream>
#include<ctime>

#include "agents.h"

Measurement::Measurement( const ymsg_t *message )
{
    set( message );
}

Measurement::Measurement( ) { };

std::string Measurement::topic() const
{
    if( _device_name != "" && _source != "" ) return _source + "@" + _device_name;
    return "";
};

void Measurement::set( const ymsg_t *message )
{
    ymsg_t *copy = ymsg_dup( (ymsg_t *)message );
    char *device_name;
    char *source;
    char *units;

    if( bios_measurement_decode(&copy, &device_name, &source, &units, &_value, &_scale, &_time ) == 0 ) {
        _device_name = device_name;
        _source = source;
        _units = units;
        if( _time <= 0 ) _time = std::time(NULL);
    } else {
        clear();
    }
    ymsg_destroy(&copy);
}

void Measurement::clear()
{
    _device_name = "";
    _source = "";
    _units = "";
    _value = 0;
    _scale = 0;
    _time = 0;
    
}

void Measurement::print()
{
    std::cout <<
        "device: " << _device_name << "\n"
        "source: " << _source << "\n" <<
        "units: " << _units << "\n" <<
        "value: " << ( _value * pow( 10, _scale ) ) << "\n";
}

