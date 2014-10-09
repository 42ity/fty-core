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

/*! \file clientinfo.h
    \brief A class for manipulating with database objects stored 
    in the table t_bios_client_info.

    \author Alena Chernikava <alenachernikava@eaton.com>
*/  
#ifndef UTILS_PERSISTENCE_CLIENTINFO_H_
#define UTILS_PERSISTENCE_CLIENTINFO_H_

#include "databasetimeobject.h"
#include "cidr.h"
#include "tntdb/error.h"

namespace utils {

namespace db {

/*
 * \brief ClientInfo is a class for representing a database entity
 * t_bios_client_info.
 */
class ClientInfo : public DataBaseTimeObject
{
    public:

        /**
         * \brief Creates a new object and specifies a connection.
         *
         * Creates a new object for the specified url in the state OS_NEW.
         *
         * \param url - a connection to the database.
         */
        ClientInfo(std::string url);

        /**
         * \brief Creates a new object with the specified ClientName and 
         * specifies a connection.
         *
         * Creates a new object with the specified ClientName for 
         * the specified url in the state OS_NEW.
         *
         * \param clientName - a name of the client who provides this information.
         * \param url        - a connection to the database.
         */
        ClientInfo(std::string url, std::string clientName);
        
        /**
         * \brief Creates a new object with the specified ClientId and 
         * specifies a connection.
         *
         * Creates a new object with the specified ClientId for 
         * the specified url in the state OS_NEW.
         *
         * \param clientId - an id of the client who provides this information.
         * \param url      - a connection to the database.
         */
        ClientInfo(std::string url,  int clientId);

        /**
         * \brief Converts all fields to string and concatenates them.
         *
         * \return Object as string.
         */
        std::string toString();
        
        /**
         * \Brief Returns an object to OS_NEW state with initial parameters.
         */
        void clear();
        
        /**
         * \brief  Selects the last information about device with deviceDiscoveredId .
         * provided by client with clientId(clientName) stored in this object.
         *
         * Takes existing clientId and deviceDiscoveredId.
         * \return Returns a number of selected rows.
         */
        unsigned int selectLastRecord();

        /**
         * \brief  Selects the last information about device with deviceDiscoveredId. 
         * provided by client with clientId.
         *
         * Takes clientId and deviceDiscoveredId as parameters.
         * In case of success (one row found) rewrite all object .
         * and put it in state OS_SELECTED. In other cases nothing to change.
         * 
         * \return Returns a number of selected rows.
         */
        unsigned int selectLastRecord(int clientId, int deviceDiscoveredId);

        /**
         * \brief  Selects the last information about device with deviceDiscoveredId .
         * provided by client with clientName.
         *
         * Takes clientName and deviceDiscoveredId as parameters.
         * In case of success (one row found) rewrite all object .
         * and put it in state OS_SELECTED. In other cases nothing to change.
         * 
         * \return Returns a number of selected rows.
         */
        unsigned int selectLastRecord(std::string clientName, int deviceDiscoveredId);
        
        /**
         * \brief Gets an id of discovered device which this info regards.
         */
        int getDeviceDiscoveredId();

        /**
         * \brief Gets an id of client that gathered that info.
         */
        int getClientId();

        /**
         * \brief Gets a blob data.
         */
        std::string getBlobData();

        /**
         * \brief Gets a client name that gathered that info.
         */
        std::string getClientName();

        /**
         * \brief Sets a clientId (clientName is selected from DB automatically).
         */
        void  setClientId(int clientId);
        
        /**
         * \brief Sets a DeviceDiscoveredId.
         */
        void  setDeviceDiscoveredId(int deviceDiscoveredId);

        /**
         * \brief Sets a new blobData.
         */
        void  setBlobData(std::string blobData);
        
        ~ClientInfo();

        /* reads History from db for specified client ,and deviceDiscovered*/
/* NOT IMPLEMENTED 
        // returns Delailed Info for existing clientID and deviceDiscoveredID
        std::vector<ClientInfo> getHistoryClientInfo(dateType date_type, time_t date, time_t date2 = time(NULL) );         
        // returns Detailed Info for clientName and deviceDiscoveredIp
        static std::vector<ClientInfo> getHistoryClientInfo
                (std::string clientName, CIDRAddress deviceDiscoveredIp, dateType date_type, time_t date, time_t date2 = time(NULL)); 
        static std::vector<ClientInfo> getHistoryClientInfo
                (std::string clientName, std::string deviceDiscoveredIp, dateType date_type, time_t date, time_t date2 = time(NULL)); 
        // returns Detailed Info for existing clientID and deviceDiscoveredID       
        std::vector<ClientInfo> getHistoryClientInfo(int n = 0); 
        // returns Detailed Info for clientName and ClientInfoIp       
        static std::vector<ClientInfo> getHistoryClientClientInfo(std::string clientName, CIDRAddress deviceDiscoveredIp, int n = 0); 
        static std::vector<ClientInfo> getHistoryClientClientInfo(std::string clientName, std::string deviceDiscoveredIp, int n = 0); 
*/
        //------------------//
        /* reads History from db for specified client*/
/* NOT IMPLEMENTED
        // returns Detailed info for existing clientID
        std::vector<ClientInfo> getHistoryClient(dateType date_type, time_t date, time_t date2 = time(NULL));
        // returns Detailed info for ClientName
        static std::vector<ClientInfo> getHistoryClient(std::string clientName, dateType date_type, time_t date, time_t date2 = time(NULL));
        // returns Detailed info for existing clientId
        std::vector<ClientInfo> getHistoryClient(int n = 0); 
        // returns Detailed info for ClientName
        static std::vector<ClientInfo> getHistoryClient(std::string clientName, int n = 0);
*/
        //-------------------//
        /* reads History from db record for specified deviceDiscovered*/
/* NOT IMPLEMENTED
        // returns Detailed Info for existing deviceDiscoveredId
        std::vector<ClientInfo> getHistoryClientInfo(dateType date_type, time_t date, time_t date2 = time(NULL));
        // returns Detailed Info for deviceDiscoveredIp
        std::vector<ClientInfo> getHistoryClientInfo(CIDRAddress deviceDiscoveredIp, dateType date_type, time_t date, time_t date2 = time(NULL));
        std::vector<ClientInfo> getHistoryClientInfo(std::string deviceDiscoveredIp, dateType date_type, time_t date, time_t date2 = time(NULL));
        // returns Detailed Info for existing deviceDiscoveredId
        std::vector<ClientInfo> getHistoryClientInfo(int n = 0);
        // returns Detailed Info for deviceDiscoveredIp
        std::vector<ClientInfo> getHistoryClientInfo(CIDRAddress deviceDiscoveredIp, int n = 0); // takes existing deviceDiscoveredID
        std::vector<ClientInfo> getHistoryClientInfo(std::string deviceDiscoveredIp, int n = 0); // takes existing deviceDiscoveredID
*/
        /**
         * \brief Selects from DB a client info by ID. Rewrites object.
         *
         * If client info was found:
         * -selects exactly one row.
         * -state changed to OS_SELECTED.
         *
         * If client info was not found:
         * -selects nothing.
         * -everything remains the same.
         *  
         * \param id - id of the row in database.
         *  
         * \return A number of rows selected.
         */
        unsigned int selectById(int id);
        
    protected:
        
        /**
         * \brief Checks if client was identified (by ID or by Name).
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

        /**
         * \brief Selects a timestempt for stored ID.
         *
         *  All necessary pre-actions are made in selectTimestamp.
         *
         * \return A Number of rows affected.
         */
        unsigned int db_select_timestamp();

        /**
         * \brief Selects a clientname for current clientid.
         *
         * \return Returns true if client with specified ID was found.
         */
        bool selectClientName(int clientId);
        
    private:

        /**
         * \brief Returns private fields of this object to initial state.
         */
        void clear_this();
        
        /////////////////////////////////
        /* fields from the DB*/
        /////////////////////////////////

        /**
         * \brief Id of the client who gathered this info.
         */
        int _clientId;

        /**
         * \brief An id of the discovered device which was scanned.
         */
        int _discoveredDeviceId;

        /**
         * \brief an information in unknown format. We don't care.
         */
        std::string _blobData;

        /////////////////////////////////
        /* calculateed fields */
        ////////////////////////////////

        /**
         * \brief Name of the client who gathered this info.
         *
         * Calculates automatically. Cannot be changed manually.
         */
        std::string _clientName;

}; // end of the class

} // namespace db

}  // end of namespace utils

#endif // CLIENTINFO_H_
