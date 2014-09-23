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

/*! \file persistence.h
    \brief Class for manipulating with database

    While all our modules must behave well and must not do 
    unnecessary actions, for all specific purposes 
    specific functions are prepared.

    It is assumed that all IP addresses would be written in IPv6 format
    
    \author Alena Chernikava <alenachernikava@eaton.com>
*/  
#pragma once    
    
#ifndef PERSISTENCE_H_
#define PERSISTENCE_H_

#define MODULE_ADMIN "admin"

#include <vector>
#include <string>
#include <iostream>
#include <ctime>
#include <tntdb/connection.h>
#include <tntdb/connect.h>
#include "cidr.h"
#include "databaseobject.h"

namespace utils {

//toString(ObjectState objectstate);

class Ip;
class Netmon : public DataBaseObject
{
    public:

        Netmon(std::string url);

        ~Netmon();

        void setMac(std::string mac);

        void setMask(int mask);

        void setIp(std::string ip);

        void setIp(CIDRAddress ip);

        void setCommandId(int commandId);

        void setCommandName(std::string name);

        std::string getMac();

        int getMask();

        CIDRAddress getIpCIDR();

        std::string getIp();

        CIDRAddress getNetworkCIDR();

    protected:

    private:
        
        std::string _mac;

        int _mask;

        CIDRAddress _ip;

        time_t _date;

        int _commandId;
};

} // namespace utils

#endif // PERSISTENCE_H_
