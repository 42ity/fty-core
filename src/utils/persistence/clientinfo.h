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
    \brief Basic Class for manipulating with database object t_bios_client_info

    \author Alena Chernikava <alenachernikava@eaton.com>
*/  
#ifndef CLIENTINFO_H_
#define CLIENTINFO_H_

#include "databasetimeobject.h"
#include "cidr.h"
#include "tntdb/error.h"

namespace utils {

class ClientInfo : public DataBaseTimeObject
{
    public:

        /**
         * \brief Creates a new object with specified connection
         */
        ClientInfo(std::string url);

        /** 
         * \brief creates a new object with specified connection for a client clientName
         */
        ClientInfo(std::string url, std::string clientName);
        
        /** 
         * \brief creates a new object with specified connection for a client clientId
         */
        ClientInfo(std::string url, unsigned int clientId);

        /**
         * \brief Returns all fields as string
         */
        std::string toString();
        
        /**
         * \Brief Returns an object to OS_NEW state with initial parameters
         */
        void clear();
        
        /**
         * \brief  Selects the last information about device with deviceDiscoveredId 
         * provided by client with clientId(clientName) stored in this object
         *
         * Takes existing clientId and deviceDiscoveredId
         * \return Returns a number of selected rows
         */
        unsigned int selectLastRecord();

        /**
         * \brief  Selects the last information about device with deviceDiscoveredId 
         * provided by client with clientId
         *
         * Takes clientId and deviceDiscoveredId as parameters.
         * In case of success (one row found) rewrite all object 
         * and put it in state OS_SELECTED. In other cases nothing to change.
         * 
         * \return Returns a number of selected rows
         */
        unsigned int selectLastRecord(int clientId, int deviceDiscoveredId);

        /**
         * \brief  Selects the last information about device with deviceDiscoveredId 
         * provided by client with clientName
         *
         * Takes clientName and deviceDiscoveredId as parameters.
         * In case of success (one row found) rewrite all object 
         * and put it in state OS_SELECTED. In other cases nothing to change.
         * 
         * \return Returns a number of selected rows
         */
        unsigned int selectLastRecord(std::string clientName, int deviceDiscoveredId);
        
        /**
         * \brief Get an id of deviceDiscovered
         */
        unsigned int getDeviceDiscoveredId();

        /**
         * \brief Get an id of client that gathered that info
         */
        unsigned int getClientId();

        /**
         * \brief Get a blob data
         */
        std::string getBlobData();

        /**
         * \brief Get a clientname
         */
        std::string getClientName();

        /**
         * \brief set a clientId, clientName is selected from DB
         */
        void  setClientId(int clientId);
        
        /**
         * \brief set a DeviceDiscoveredId
         */
        void  setDeviceDiscoveredId(int deviceDiscoveredId);

        /**
         * \brief set a new blobData
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

    protected:
        
        /**
         * \brief TODO if need
         */
        bool check(){};

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
        
        /**
         * \brief Selects a timestempt for stored ID.
         *
         *  All necessary pre-actions are made in selectTimestampt
         */
        unsigned int db_select_timestampt();

        /**
         * \brief Selects a clientname for current clientid
         *
         * \return Returns true if client with specified ID was found
         */
        bool selectClientName(unsigned int clientId);
        
    private:
        /////////////////////////////////
        /* fields from the DB*/
        /////////////////////////////////

        /**
         * \brief Id of the client who gathered this info
         */
        int _clientId;

        /**
         * \brief An id of the discovered device which was scanned
         */
        int _deviceDiscoveredId;

        /**
         * \brief an information in unknown format. We don't care.
         */
        std::string _blobData;

        /////////////////////////////////
        /* calculateed fields */
        ////////////////////////////////

        /**
         * \brief Name of the client who gathered this info
         *
         * Calculates automatically. Cannot be changed manually.
         */
        std::string _clientName;
};

} // end of namespace utils

#endif // CLIENTINFO_H_
