#pragma once

#ifndef MODULEINFO_H_
#define MODULEINFO_H_


#include "databaseobject.h"
#include "cidr.h"


namespace utils {

class ModuleInfo : public DataBaseObject
{
    public:

        /**
         * \brief Creates a new object with specified connection
         */
        ModuleInfo(std::string url);

        /** 
         * \brief creates a new object with specified connection for a client clientName
         */
        ModuleInfo(std::string url, std::string clientName);

        std::string toString();
        
        /*Must fill also name*/
        void  setModuleId(int clientId);

        void  setClientInfoId(int deviceDiscoveredId);

        void  setDate(time_t date);

        void  setBlobData(std::string blobData);

        int getModuleId();

        int getClientInfoId();

        time_t getDate();

        std::string getBlobData();

        ~ModuleInfo();

        std::string getName();

        /* reads name from DB , by clientId */
        void getNameDB();

        /* changes clients name by finding in DB apropriate id and return true if it finds or false if it doesn't find
         * changes client id*/
        bool setName(std::string name);

        /* changes and id and load from DB*/
        bool loadById(int id);

        /* reloads info from DB by existed id*/
        bool reload();

        unsigned int dbSave();

        unsigned int dbUpdate();

        unsigned int dbDelete();

        /**
         * \brief  Selects last information about device deviceDiscoveredId 
         * provided by client with clientId(clientName) stored in this object
         *
         * Takes existing clientId and deviceDiscoveredId
         * \return Returns a number of selected rows
         */
        unsigned int selectLastRecord();

        /**
         * \brief  Selects last information about device deviceDiscoveredId 
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
         * \brief  Selects last information about device deviceDiscoveredId 
         * provided by client with clientName
         *
         * Takes clientName and deviceDiscoveredId as parameters.
         * In case of success (one row found) rewrite all object 
         * and put it in state OS_SELECTED. In other cases nothing to change.
         * 
         * \return Returns a number of selected rows
         */
        unsigned int selectLastRecord(std::string clientName, int deviceDiscoveredId);

        bool selectLastRecord(std::string ModuleName, CIDRAddress ip); // load info from client-name for deviceDiscovered, that has ip

        bool selectLastRecord(std::string ModuleName, std::string ip); // load info from client-name for deviceDiscovered, that has ip
        //-----------------//
        /* reads History from db for specified client ,and deviceDiscovered*/

        // returns Delailed Info for existing clientID and deviceDiscoveredID
        std::vector<ModuleInfo> getHistoryModuleClientInfo(dateType date_type, time_t date, time_t date2 = time(NULL) );         
        // returns Detailed Info for clientName and deviceDiscoveredIp
        static std::vector<ModuleInfo> getHistoryModuleClientInfo
                (std::string clientName, CIDRAddress deviceDiscoveredIp, dateType date_type, time_t date, time_t date2 = time(NULL)); 
        static std::vector<ModuleInfo> getHistoryModuleClientInfo
                (std::string clientName, std::string deviceDiscoveredIp, dateType date_type, time_t date, time_t date2 = time(NULL)); 
        // returns Detailed Info for existing clientID and deviceDiscoveredID       
        std::vector<ModuleInfo> getHistoryModuleClientInfo(int n = 0); 
        // returns Detailed Info for clientName and ClientInfoIp       
        static std::vector<ModuleInfo> getHistoryModuleClientInfo(std::string clientName, CIDRAddress deviceDiscoveredIp, int n = 0); 
        static std::vector<ModuleInfo> getHistoryModuleClientInfo(std::string clientName, std::string deviceDiscoveredIp, int n = 0); 

        //------------------//
        /* reads History from db for specified client*/

        // returns Detailed info for existing clientID
        std::vector<ModuleInfo> getHistoryModule(dateType date_type, time_t date, time_t date2 = time(NULL));
        // returns Detailed info for ModuleName
        static std::vector<ModuleInfo> getHistoryModule(std::string clientName, dateType date_type, time_t date, time_t date2 = time(NULL));
        // returns Detailed info for existing clientId
        std::vector<ModuleInfo> getHistoryModule(int n = 0); 
        // returns Detailed info for ModuleName
        static std::vector<ModuleInfo> getHistoryModule(std::string clientName, int n = 0);

        //-------------------//
        /* reads History from db record for specified deviceDiscovered*/

        // returns Detailed Info for existing deviceDiscoveredId
        std::vector<ModuleInfo> getHistoryClientInfo(dateType date_type, time_t date, time_t date2 = time(NULL));
        // returns Detailed Info for deviceDiscoveredIp
        std::vector<ModuleInfo> getHistoryClientInfo(CIDRAddress deviceDiscoveredIp, dateType date_type, time_t date, time_t date2 = time(NULL));
        std::vector<ModuleInfo> getHistoryClientInfo(std::string deviceDiscoveredIp, dateType date_type, time_t date, time_t date2 = time(NULL));
        // returns Detailed Info for existing deviceDiscoveredId
        std::vector<ModuleInfo> getHistoryClientInfo(int n = 0);
        // returns Detailed Info for deviceDiscoveredIp
        std::vector<ModuleInfo> getHistoryClientInfo(CIDRAddress deviceDiscoveredIp, int n = 0); // takes existing deviceDiscoveredID
        std::vector<ModuleInfo> getHistoryClientInfo(std::string deviceDiscoveredIp, int n = 0); // takes existing deviceDiscoveredID


    protected:
        
        /**
         * \brief TODO if need
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
         * \brief date of gathering information
         *
         * it is a date of inserting the row in the table
         * it is not valid, until the state is OS_SELECTED.
         */
        time_t _date;

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

#endif // MODULEINFO_H_
