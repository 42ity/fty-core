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
 
#ifndef SRC_AGENTS_TPOWER_TP_UNIT_H__
#define SRC_AGENTS_TPOWER_TP_UNIT_H__

#include <map>
#include <string>
#include <ctime>

#include "alert-measurement.h"
#include "ymsg.h"

//! \brief class representing total power calculation unit (rack or DC)
class TPUnit {
 public:
    //\! \brief calculate total realpower
    Measurement realpower();
    
    //\! \brief get set unit name
    std::string name() { return _name; };
    void name(const std::string &name) { _name = name; };
    void name(const char *name) { _name =  name ? name : ""; };
    
    //\! \brief returns true if at least one measurement of all included powerdevices is unknown  
    bool realpowerIsUnknown() const;
    //\! \brief returns true if totalpower can be calculated.
    bool realpowerIsKnown() const { return ! realpowerIsUnknown(); }
    
    //\! \brief add powerdevice to unit 
    void addPowerDevice(const std::string &device);

    //\! \brief save new received measurement
    void setMeasurement(const Measurement &M);
    
    //! \brief returns true if measurement is changend and we should advertised
    bool changed() { return _changed; }

    //! \brief set/clear changed status
    void changed(bool newStatus);

    //! \brief returns true if measurement should be send (changed is true or we did not send it for long time)
    bool advertise();

    //! \brief set timestamp of the last publishing moment
    void advertised();

    //! \brief create ymsg meeasurement message
    ymsg_t *measurementMessage();
 protected:
    //! \brief measurement status
    bool _changed = false;

    //! \brief measurement timestamp
    time_t _timestamp;

    //! \brief list of measurements for included devices
    std::map< std::string, Measurement > _powerdevices;

    //! \brief unit name
    std::string _name;
};

#endif // SRC_AGENTS_TPOWER_TP_UNIT_H__
