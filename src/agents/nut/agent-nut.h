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

/*! \file agent-nut.h
    \brief 
    \author Tomas Halman <tomashalman@eaton.com>
*/

#ifndef SRC_AGENTS_NUT_NUT_AGENT_H__
#define SRC_AGENTS_NUT_NUT_AGENT_H__

#include "bios_agent++.h"
#include "nut-driver.h"

class NUTAgent : public BIOSAgent {
 public:
    explicit NUTAgent( const char *agentName ) :BIOSAgent( agentName ) {  };
    explicit NUTAgent( const std::string &agentName ) :BIOSAgent( agentName ) { };
    void onPoll();
    void onStart();
 protected:
    std::string physicalQuantityShortName(const std::string &longName);
    std::string physicalQuantityToUnits(const std::string &quantity);
    void advertiseInventory();
    void advertisePhysics();
    drivers::nut::NUTDeviceList _deviceList;
    std::time_t _measurementTimestamp = 0;
    std::time_t _inventoryTimestamp = 0;
    std::map<std::string, std::string> _units = {
        { "temperature", "C" },
        { "realpower",   "W" },
        { "voltage",     "V" },
        { "current",     "A" },
        { "load",        "%" },
        { "charge",      "%" },
        { "frequency",   "Hz"},
        { "power",       "W" },
        { "runtime",     "s" },
    };
};

#endif // SRC_AGENTS_NUT_NUT_AGENT_H__

