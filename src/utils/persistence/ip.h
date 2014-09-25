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

/*! \file ip.h
    \brief Class for manipulating with the database objects stored 
    in the table t_bios_discovered_ip.
    
    \author Alena Chernikava <alenachernikava@eaton.com>
*/ 

#ifndef IP_H_
#define IP_H_

#include "cidr.h"
#include "databasetimeobject.h"

class DeviceDiscovered;

namespace utils{
    
/*
 * \brief IP is a class for representing a database entity
 * t_bios_descovered_ip.
 */

class Ip :public DataBaseTimeObject
{

    public:
        
        Ip(std::string url);

        Ip(std::string url, std::string ip);

        ~Ip(){};
        
        void setIp(std::string ip);

        std::string getIp();

        CIDRAddress getIpCIRD();

        void setIp(CIDRAddress ip);

        void setDeviceDiscoveredId(int deviceDiscoveredId);

        int getDeviceDiscoveredId();

        std::vector<Ip> getOtherIps();

        unsigned int selectById(unsigned int n);
        
        void clear(){};
        /**
         * \brief Selects the last information about Ip and returns it as Ip object in state OS_SELECTED.
         */
        static Ip* getLastInfo(std::string url, std::string ip);

        /*reads from Db, doesnt store it anywhere*/
        DeviceDiscovered getDeviceDiscovered();

        std::vector<DeviceDiscovered> getIpHistory(dateType date_type, time_t date, time_t date2 = time(NULL));

        static std::vector<DeviceDiscovered> getIpHistory(std::string ip, dateType date_type, time_t date, time_t date2 = time(NULL));

        static std::vector<DeviceDiscovered> getIpHistory(CIDRAddress ip, dateType date_type, time_t date, time_t date2 = time(NULL));

        std::vector<DeviceDiscovered> getIpHistory(int n = 0);

        static std::vector<DeviceDiscovered> getIpHistory(std::string ip, int n = 0);

        static std::vector<DeviceDiscovered> getIpHistory(CIDRAddress ip, int n = 0);

    protected:
        /**
         * \brief Checks the name length
         *
         * TODO add more checks if needed
         */
        bool check();

        /**
         *  \brief inserts a row.
         *
         *  All necessary pre-actions are made in dbsave
         */
        unsigned int db_insert();
        
        /**
         *  \brief updates a row.
         *
         *  All necessary pre-actions are made in dbsave
         */
        unsigned int db_update();
        
        /**
         *  \brief deletes a row.
         *
         *  All necessar
         */
        unsigned int db_delete();
    private:
        
        CIDRAddress _ip;

        int _deviceDiscoveredId;

}; // end of the class

}  // namespace utils

#endif //IP_H_
