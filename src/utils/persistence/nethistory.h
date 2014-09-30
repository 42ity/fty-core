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

/*! \file NetHistory.h
    \brief Class for manipulating with database table t_bios_net_history

    \author Alena Chernikava <alenachernikava@eaton.com>
*/  
#ifndef NETHISTORY_H_
#define NETHISTORY_H_

#include <string>
#include <ctime>
#include <tntdb/connection.h>
#include <tntdb/connect.h>
#include "cidr.h"
#include "databasetimeobject.h"

namespace utils {

class NetHistory : public DataBaseTimeObject
{
    public:
         
        /**
         * \brief Creates a new object and specifies a connection.
         *
         * Creates a new object for the specified url in state OS_NEW.
         */
        NetHistory(std::string url);

        /**
         * \brief Returns an object to initial state. It doesn't manipulate with database.
         */
        void clear();

        /**
         * \brief Returns all fields as string
         */
        std::string toString();

        ~NetHistory();

        void setMac(std::string mac);

        void setMask(int mask);

        void setIp(std::string ip);

        void setIp(CIDRAddress ip);

        void setCommand(char command);

        std::string getMac();

        int getMask();

        CIDRAddress getIp();

        char getCommand();

        CIDRAddress getNetworkCIDR();

        unsigned int selectById(int id);

    protected:
        
        void clear_this();

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
        
        /**
         * \brief Selects a timestamp for stored ID.
         *
         * Internal method.
         */
        unsigned int db_select_timestamp();

    private:
        
        std::string _mac;

        int _mask;

        CIDRAddress _ip;

        char _command;

}; // end of the class

}  // end of namespace utils

#endif // NETHISTORY_H_
