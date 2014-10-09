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
    \brief Class for manipulating with the database objects stored 
    in the table t_bios_discovered_device.

    \author Alena Chernikava <alenachernikava@eaton.com>
*/  
#ifndef UTILS_PERSISTENCE_DEVICE_DISCOVERED_H_
#define UTILS_PERSISTENCE_DEVICE_DISCOVERED_H_

#define MODULE_ADMIN "admin"

#include "databaseobject.h"
#include "cidr.h"
#include "ip.h"
#include "client.h"
#include <ctime>
#include "clientinfo.h"

namespace utils{

namespace db {

/*
 * \brief DeviceDiscovered is a class for representing a database entity
 * t_bios_discovered_device.
 */
class DeviceDiscovered:  public DataBaseObject {
    
    public:
    
        /**
         * \brief Creates a new object and specifies a connection.
         *
         * Creates a new object for the specified url with name = "unknown" 
         * in state OS_NEW.
         *
         * \param url - connection to the database.
         */
        DeviceDiscovered(std::string url);
    
        /**
         * \brief Creates a new object with specified name and specifies a 
         * connection.
         *
         * Creates a new object for the specified url with name = name in 
         * state OS_NEW.
         *
         * \param url  - connection to the database.
         * \param name - name of discovered device.
         */
        DeviceDiscovered(std::string url, std::string name);
    
        /**
         * \brief Converts all fields to string and concatenates them.
         *
         * \return Object as string.
         */
        std::string toString();
       
        /**
         * \brief Selects from the DB discovered devices by name.
         *
         * Selects from DB discovered devices by name and returns a set of 
         * matched discovered devices in a vector. 
         * All elements there would be in state OS_SELECTED.
         
         * \param url  - connection to the database.
         * \param name - name of discovered device.
         *
         * \return a vector of devices.
         */
        static std::vector<DeviceDiscovered> 
                    selectByName(std::string url, std::string name);
       
        /**
         * \brief Selects from DB a discovered device by ID. Rewrites this object.
         *
         * If discovered device was found:
         * -selects exactly one row.
         * -state is changed to OS_SELECTED.
         *
         * If discovered device was not found:
         * -selects nothing.
         * -everything remains the same.
         *
         * \param id - id of discovered device.
         *
         * \return A number of rows selected.
         */
        unsigned int selectById(int id);
        
        /**
         * \brief Selects a discovered device by Ip.
         *
         * Selects the discovered device last assigned to the specified Ip.
         *
         * \param ip - ip of device.
         *
         * \return A number of rows selected.
         */
        unsigned int selectByIp(CIDRAddress ip);
        
        /**
         * \brief Selects a discovered device by Ip.
         *
         * Selects thediscovered device last assigned to the specified Ip.
         *
         * \param ip - ip of device.
         *
         * \return A number of rows selected.
         */
        unsigned int selectByIp(std::string ip);
        
        /**
         * \brief Selects the last information about this discovered device.
         * 
         * In case of state OS_NEW/OS_DELETED there is nothing to do and it returns 0.
         *
         * \return A number of rows selected.
         */
        ClientInfo *selectLastDetailInfo();

        /**
         * \brief Get a discovered device's name.
         *
         * \return A name od discovered device.
         */
        std::string getName();
        
        /**
         * \brief Sets a new name for the object.
         *
         * If state is OS_DELETED then do nothing.
         * If newname = oldname then do nothing.
         * If state is OS_SELECTED and newname != oldname.
         *  than state is changed to OS_UPDATED.
         *
         *  \param name - new name of the discovered device.
         */ 
        void setName(std::string name);
    
        /* NOT IMPLEMENTED
        std::vector<ClientInfo> getHisroty(time_t date, dateType date_type);
    
        std::vector<ClientInfo> getHistory(int n = 0);
    
        // for this deviceDiscoveredId
        std::vector<Ip> getIps();
        
        std::vector<Ip> getIpsv4();
    
        std::vector<Ip> getIpsv6();
     
        Ip addIp(std::string ip);
    
        Ip addIp(CIDRAddress ip);
           */
        //NOT IMPLEMENTED
        /* merges this deviceDiscovered with deviceDiscovered and 
         * result is in this.
         * TODO the deviceDiscovered would be deleted. How to solve .
         * history dependences? delete or update with new comp_id or join
         * with some dummy element.
         
        void mergeWithDeviceDiscovered(DeviceDiscovered deviceDiscovered);
    
        void mergeWithDeviceDiscoveredList
                (std::vector<DeviceDiscovered> deviceDiscovereds);
         */


        ~DeviceDiscovered();

        /**
         * \brief Get an Id of the client responsible for detailed info 
         * about this discovered device.
         *
         * \return Id of the client responsible for detailed info.
         */
        int getDetailedClientId();

        /**
         * \brief Get a name of the client responsible for detailed info 
         * about this discovered device.
         *
         * \return Name of the client responsible for detailed info.
         */
        static std::string getDetailedClientName();

        /**
         * \brief Returns an object to OS_NEW state with initial parameters.
         * 
         */
        void clear();
        
        //TODO at the end, instead of this method use method setDeviceType(DeviceType deviceType)
        //                                                   setDeviceType(std::string deviceTypeName)
        //it can garantee that this field is invalid only id deviceTypeId = -1.
        /**
         * \brief Sets a new device type for the object.
         *
         * If state is OS_DELETED then do nothing.
         * If newtype = oldtype then do nothing.
         * If state is OS_SELECTED and newtype != oldtype.
         *  than state is changed to OS_UPDATED.
         *
         *  \param deviceTypeId - new deviceTypeid of the discovered device.
         */ 
        void setDeviceTypeId(int deviceTypeId);
        
        /**
         * \brief Get an Id of discovered device type.
         *
         * \return Id of discovered device type.
         */
        int getDeviceTypeId();

    protected:
        
        /**
         * \brief Checks the name length.
         *
         * \return true if check was successful.
         */
        bool check();
        
        /**
         * \brief Internal method for insert.
         *
         * All necessary pre-actions are made in dbsave.
         *
         * \return A Number of rows affected.
         */
        unsigned int db_insert();
        
        /**
         * \brief Internal method for update.
         *
         * All necessary pre-actions are made in dbsave.
         *
         * \return A Number of rows affected.
         */
        unsigned int db_update();

        /**
         * \brief Internal method for delete.
         * 
         * All necessary pre-actions are made in dbdelete.
         *
         * \return A Number of rows affected.
         */
        unsigned int db_delete();
        
    private:
        
        /**
         * \brief Returns private fields of this object to initial state.
         */
        void clear_this();
       
        /**
         * \brief An Id of the client which is responsible for detailed info.
         *
         * This field is selected from DB in constructor and user can not 
         * modify it, but could access its value throgh get-method.
         */
        int _clientId;
        
        /**
         * \brief A name of the client which is responsible for detailed info.
         */
        static std::string _clientName;
        
        /**
         * \brief A name of the discovered device.
         */
        std::string _name;
    
        /**
         * \brief Id of discovered device type.
         *
         * id = -1 means it is invalid.
         */
        int _deviceTypeId;

}; // end of the class

} // namespace db

}  // end of namaspace utils


#endif // UTILS_PERSISTENCE_DEVICE_DISCOVERED_H_
