#pragma once

#ifndef IP_H_
#define IP_H_

#include "cidr.h"
#include "databaseobject.h"

class DeviceDiscovered;

namespace utils{
    

class Ip :public DataBaseObject
{

    public:

        Ip(std::string url);

        Ip(std::string url, std::string ip);

        ~Ip();
        
        void setIp(std::string ip);

        std::string getIp();

        CIDRAddress getIpCIRD();

        void setIp(CIDRAddress ip);

        void setDeviceDiscoveredId(int deviceDiscoveredId);

        int getDeviceDiscoveredId();

        time_t getDate();

        void setDate(time_t date);

        unsigned int dbSave();

        unsigned int dbDelete();
        
        std::vector<Ip> getOtherIps();

        unsigned int selectById(unsigned int n){};
        
        void clear(){};
        /**
         * \brief Selects the last information about Ip and returns it as Ip object in state OS_SELECTED.
         */
        static Ip* getLastInfo(std::string url, std::string Ip);

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
        bool check(){};

        /**
         *  \brief inserts a row.
         *
         *  All necessary pre-actions are made in dbsave
         */
        unsigned int db_insert(){};
        
        /**
         *  \brief updates a row.
         *
         *  All necessary pre-actions are made in dbsave
         */
        unsigned int db_update(){};
        
        /**
         *  \brief deletes a row.
         *
         *  All necessar
         */
        unsigned int db_delete(){};
    private:
        
        CIDRAddress _ip;

        int _deviceDiscoveredId;

        time_t _date;

};

} // namespace utils

#endif //IP_H_
