/*
Copyright (C) 2014 Eaton

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/*! \file   measurement.h
    \brief  Object representing one measurement
    \author Tomas Halman <TomasHalman@Eaton.com>
*/

#ifndef SRC_AGENTS_TPOWER_MEASUREMENT_H__
#define SRC_AGENTS_TPOWER_MEASUREMENT_H__

#include <string>
#include <ctime>

#include "ymsg.h"

class Measurement {
 public:
    Measurement( const ymsg_t *message );
    Measurement();

    void set( const ymsg_t *message );
    void clear( );
    std::string topic() const;
    std::string deviceName() const { return _device_name; };
    std::string source() const { return _source; };
    std::string units() const { return _units; };
    void units(const std::string &U) { _units = U; };
    int32_t value() const { return _value; };
    int32_t scale() const { return _scale; };
    int64_t time() const { return _time; };
    void setTime() { _time = std::time(NULL); };
    void setTime(time_t time ) { _time = time; };
    void setDeviceName(const std::string &deviceName ) { _device_name = deviceName; };
    void setSource(const std::string &source) { _source = source; };
    void setUnits(const std::string &units) { _units = units; };
    void setScale(int32_t scale) { _scale = scale; };
    void setValue(int32_t value) { _value = value; };
    void print() const;
    Measurement& operator=( const ymsg_t *message ) { set( message ); return *this; }
    Measurement& operator+=( const Measurement &rhs );
 protected:
    std::string _device_name;
    std::string _source;
    std::string _units;
    int32_t _value = 0;
    int32_t _scale = 0;
    int64_t _time = 0;
};

inline bool operator==( const Measurement &lhs, const Measurement &rhs ) {
    return ( lhs.units() == rhs.units() &&
             lhs.value() == rhs.value() &&
             lhs.scale() == rhs.scale() );
}

inline bool operator!=( const Measurement &lhs, const Measurement &rhs ) {
    return ! ( lhs == rhs );
}

#endif // SRC_AGENTS_TPOWER_MEASUREMENT_H__

