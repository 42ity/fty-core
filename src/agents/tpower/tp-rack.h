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
    \brief Object representing one rack or DC
    \author Tomas Halman <tomashalman@eaton.com>
*/
 
#ifndef SRC_AGENTS_TPOWER_TP_RACK_H__
#define SRC_AGENTS_TPOWER_TP_RACK_H__

#include <map>
#include <string>
#include <ctime>

#include "alert-measurement.h"
#include "ymsg.h"

class TPRack {
 public:
    Measurement realpower();
    std::string name() { return _name; };
    void name(const std::string &name) { _name = name; };
    void name(const char *name) { _name =  name ? name : ""; };
    bool realpowerIsUnknown() const;
    bool realpowerIsKnown() const { return ! realpowerIsUnknown(); }
    void addPowerDevice(const std::string &device);
    void setMeasurement(const Measurement &M);
    bool changed() { return _changed; }
    void changed(bool newStatus);
    bool advertise();
    void advertised();
    ymsg_t *measurementMessage();
 protected:
    bool _changed = false;
    time_t _timestamp;
    std::map< std::string, Measurement > _powerdevices;
    std::string _name;
};

#endif // SRC_AGENTS_TPOWER_TP_RACK_H__
