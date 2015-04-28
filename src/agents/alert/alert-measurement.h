/*
Copyright (C) 2014 Eaton
 
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*! \file alert_measurement.h
    \brief Object representing one measurement
    \author Tomas Halman <tomashalman@eaton.com>
*/
 
#ifndef SRC_AGENTS_ALERT_ALERT_MEASUREMENT_H__
#define SRC_AGENTS_ALERT_ALERT_MEASUREMENT_H__

#include <string>

#include "ymsg.h"

typedef enum {
    ALARM_STATE_UNKNOWN = -1,
    ALARM_STATE_NO_ALARM,
    ALARM_STATE_ONGOING_ALARM
} alarm_state_t;

typedef enum {
    ALARM_SEVERITY_UNKNOWN = 0,
    ALARM_SEVERITY_P1,
    ALARM_SEVERITY_P2,
    ALARM_SEVERITY_P3,
    ALARM_SEVERITY_P4,
    ALARM_SEVERITY_P5
} alarm_severity_t;


#define NO_ALARM_PUBLISH_INTERVAL 20
#define ONGOING_ALARM_PUBLISH_INTERVAL 5

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
    int32_t value() const { return _value; };
    int32_t scale() const { return _scale; };
    int64_t time() const { return _time; };
    void print() const;
    Measurement& operator=( const ymsg_t *message ) { set( message ); return *this; }
 protected:
    std::string _topic;
    std::string _device_name;
    std::string _source;
    std::string _units;
    int32_t _value = 0;
    int32_t _scale = 0;
    int64_t _time = 0;
};

#endif // SRC_AGENTS_ALERT_ALERT_MEASUREMENT_H__

