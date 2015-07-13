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

/*! \file agent-tpower.h
    \brief 
    \author Tomas Halman <tomashalman@eaton.com>
*/
 
#ifndef SRC_AGENTS_TPOWER_AGENT_TPOWER_H__
#define SRC_AGENTS_TPOWER_AGENT_TPOWER_H__

#include "bios_agent++.h"

#include <map>
#include <vector>
#include <string>

#include "tp-unit.h"

// TODO: read this from configuration (once in 5 minutes now (300s))
#define TPOWER_MEASUREMENT_REPEAT_AFTER 300
// TODO: read this from configuration (check with upsd ever 5s)
#define TPOWER_POLLING_INTERVAL  5000


class TotalPowerAgent : public BIOSAgent {
 public:
    explicit TotalPowerAgent( const char *agentName ) :BIOSAgent( agentName ) {  };
    explicit TotalPowerAgent( const std::string &agentName ) :BIOSAgent( agentName ) { };

    void onStart();
    void onSend( ymsg_t **message );
    void onPoll();
 private:
    //! \brief list of racks
    std::map< std::string, TPUnit > _racks;
    //! \brief list of datacenters
    std::map< std::string, TPUnit > _DCs;
    //! \brief list of racks, affected by powerdevice
    std::map< std::string, std::string> _affectedRacks;
    //! \brief list of DCs, affected by powerdevice
    std::map< std::string, std::string> _affectedDCs;

    //! \brief read configuration from database
    bool configuration();
    //! \brief send measurement message if needed
    void sendMeasurement(std::map< std::string, TPUnit > &elements);
    //! \brief powerdevice to DC or rack and put it also in _affected* map
    void addDeviceToMap(
        std::map< std::string, TPUnit > &elements,
        std::map< std::string, std::string > &reverseMap,
        const std::string & owner,
        const std::string & device );

    //! \brief calculete polling interval (not to wake up every 5s)
    time_t getPollInterval();
};

#endif // SRC_AGENTS_TPOWER_AGENT_TPOWER_H__

