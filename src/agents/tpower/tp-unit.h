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

/*! \file   alert_measurement.h
    \brief  Object representing one rack or DC
    \author Tomas Halman <TomasHalman@Eaton.com>
*/
 
#ifndef SRC_AGENTS_TPOWER_TP_UNIT_H__
#define SRC_AGENTS_TPOWER_TP_UNIT_H__

#include <map>
#include <string>
#include <vector>
#include <ctime>

#include "alert-measurement.h"
#include "ymsg.h"

//! \brief class representing total power calculation unit (rack or DC)
class TPUnit {
 public:
    //\! \brief calculate total realpower
    Measurement summarize(const std::string & source) const;
    
    //\! \brief get set unit name
    std::string name() const { return _name; };
    void name(const std::string &name) { _name = name; };
    void name(const char *name) { _name =  name ? name : ""; };
    
    //\! \brief returns true if at least one measurement of all included powerdevices is unknown  
    bool quantityIsUnknown( const std::string &quantity ) const;
    //\! \brief returns true if totalpower can be calculated.
    bool quantityIsKnown( const std::string &quantity ) const {
        return ! quantityIsUnknown(quantity);
    }
    //\! \brief returns list of devices in unknown state
    std::vector<std::string> devicesInUnknownState(const std::string &quantity) const;

    //\! \brief add powerdevice to unit 
    void addPowerDevice(const std::string &device);

    //\! \brief save new received measurement
    void setMeasurement(const Measurement &M);
    
    //! \brief returns true if measurement is changend and we should advertised
    bool changed(const std::string &quantity) const;

    //! \brief set/clear changed status
    void changed(const std::string &quantity, bool newStatus);

    //! \brief returns true if measurement should be send (changed is true or we did not send it for long time)
    bool advertise( const std::string &quantity ) const;

    //! \brief set timestamp of the last publishing moment
    void advertised( const std::string &quantity );

    //! \brief time to next advertisement [s]
    time_t timeToAdvertisement( const std::string &quantity ) const;

    //! \brief create ymsg meeasurement message
    ymsg_t *measurementMessage( const std::string &quantity );

    //! \brief return timestamp for quantity change
    time_t timestamp( const std::string &quantity ) const;
 protected:
    //! \brief measurement status
    std::map < std::string, bool > _changed;
    //bool _changed = false;

    //! \brief measurement timestamp
    std::map < std::string, time_t> _timestamp;

    /*! \brief list of measurements for included devices
     *
     *     map---device1---map---realpower.default---Measurement
     *      |               +----realpover.input.L1--Measurement
     *      |               +----realpover.input.L2--Measurement
     *      |               +----realpover.input.L3--Measurement
     *      +----device2-...
     */
    std::map< std::string, std::map<std::string,Measurement> > _powerdevices;

    //! \brief unit name
    std::string _name;
};

#endif // SRC_AGENTS_TPOWER_TP_UNIT_H__
