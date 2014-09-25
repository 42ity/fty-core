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

/*! \file device_discovered.h
    \brief Basic Class for manipulating with database object t_bios_device_discovered

    \author Alena Chernikava <alenachernikava@eaton.com>
*/  
#ifndef DEVICE_DISCOVERED_H_
#define DEVICE_DISCOVERED_H_

#define MODULE_ADMIN "admin"

#include "databaseobject.h"
#include "cidr.h"
#include "ip.h"
#include "client.h"
#include <ctime>
#include "clientinfo.h"

namespace utils{

////////////////////////////////////////////////////////
////           DeviceDiscovered       //////////////////
////////////////////////////////////////////////////////

/*
 * \brief DeviceDiscovered is a class for representing a database entity
 * t_device_discovered
 */
class DeviceDiscovered:  public DataBaseObject {
    
    public:
    
        /**
         * \brief Creates a new object and specifies a connection.
         *
         * Creates a new object for the specified url with name = "unknown" 
         * in state OS_NEW.
         */
        DeviceDiscovered(std::string url);
    
        /**
         * \brief Creates a new object with specified name and specifies a 
         * connection.
         *
         * Creates a new object for the specified url with name = name in 
         * state OS_NEW.
         */
        DeviceDiscovered(std::string url, std::string name);
    
        /**
         * \brief Returns all fields as string
         */
        std::string toString();
       
        /**
         * \brief Selects from the DB discovered devices by name
         *
         * Select from DB discovered devices by name and returns a set of 
         * matched discovered devices in
         * vector. All elements there would have state OS_SELECTED.
         */
        static std::vector<DeviceDiscovered> 
                    selectByName(std::string url, std::string name);
       
        /**
         * \brief Select from DB discovered device by ID. Rewrites object.
         *
         * If discovered device was found:
         * -selects exactly one row
         * -state changed to OS_SELECTED
         *
         * If discovered device was not found:
         * -selects nothing
         * -everything remains the same
         *
         *  \return number of rows selected
         */
        unsigned int selectById(unsigned int id);
        
        /**
         * \brief Selects a discovered device by Ip.
         *
         * Selects the last assigned discovered device to the specified Ip.
         *
         * \return number of rows selected.
         * TODO toString doesnt work
         */
        unsigned int selectByIp(CIDRAddress ip);
        
        /**
         * \brief Selects a deviceDiscovered by Ip.
         *
         * Selects the last assigned deviceDiscovered to the specified Ip.
         *
         * \return number of rows selected.
         */
        unsigned int selectByIp(std::string ip);
        
        /**
         * \brief Selects the last information about this deviceDiscovered
         *
         * \return returns number of rows selected.
         *
         * In case of state OS_NEW there is nothing to do and it eturns 0.
         */
        ClientInfo *selectLastDetailInfo();

        /**
         * \brief Returns a deviceDiscovereds name
         */
        std::string getName();
        
        /**
         * \brief Sets a new name for the object.
         *
         * If state is OS_DELETED then do nothing
         * If newname = oldname then do nothing
         * If state is OS_SELECTED and newname != oldname
         *  than state is changed to OS_UPDATED.
         *
         *  \param name - new name of the client
         */ 
        void setName(std::string name);
    
        std::vector<ClientInfo> getHisroty(time_t date, dateType date_type);
    
        std::vector<ClientInfo> getHistory(int n = 0);
    
        /* for this deviceDiscoveredId*/
        std::vector<Ip> getIps();
        
        std::vector<Ip> getIpsv4();
    
        std::vector<Ip> getIpsv6();
    
        /*TODO get Ips as history*/
    
        Ip addIp(std::string ip);
    
        Ip addIp(CIDRAddress ip);
           
        /* merges this deviceDiscovered with deviceDiscovered and 
         * result is in this
         * TODO the deviceDiscovered would be deleted. How to solve 
         * history dependences? delete or update with new comp_id or join
         * with some dummy element
         */
        void mergeWithDeviceDiscovered(DeviceDiscovered deviceDiscovered);
    
        void mergeWithDeviceDiscoveredList
                (std::vector<DeviceDiscovered> deviceDiscovereds);
         
        ~DeviceDiscovered();

        /**
         * \brief Get an Id of the client responsible for detailed info 
         * about this deviceDiscovered
         */
        int getDetailedClientId();

        /**
         * \brief Get a name of the client responsible for detailed info 
         * about this deviceDiscovered
         */
        static std::string getDetailedClientName();

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
         *  All necessary pre-actions are made in dbdelete
         */
        unsigned int db_delete();
    
    private:
        
        /**
         * \brief An Id of the client which is responsible for detailed info
         *
         * This field is selected from DB in constructor and user can not 
         * modify it, 
         * but could recieve its value throgh get-method
         */
        int _clientId;
        
        /**
         * \brief A name of the client which is responsible for detailed info
         */
        static std::string _clientName;
        
        /**
         * \brief name of the deviceDiscovered
         */
        std::string _name;
    
        /**
         * TODO here must be all other columns in the table
         */
       
};

} // end of namaspace


#endif //DEVICE_DISCOVERED_H_
