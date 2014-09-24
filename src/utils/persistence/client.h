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

/*! \file client.h
    \brief Basic Class for manipulating with database object t_bios_client

    \author Alena Chernikava <alenachernikava@eaton.com>
*/  
#ifndef CLIENT_H_
#define CLIENT_H_

#include "databaseobject.h"

namespace utils{

class Client: public DataBaseObject
{
    public:

        Client(std::string url);

        unsigned int selectByName(std::string name);

        std::string getName();

        void setName(std::string name);
        
        unsigned int selectById(unsigned int id);

        ~Client();
        
        /**
         * \brief Selects an Id by name in the DB specified by url
         *
         * Selects an Id by name in the DB specified by url
         * \return an ID or -1 if nothing was found
         */
        static int selectId(std::string url, std::string name);

        std::string toString();
        
        /**
         * \Brief Returns an object to OS_NEW state with initial parameters
         */
        void clear();

    protected:
        
        /**
         *  \brief internal method for insert 
         */
        unsigned int db_insert();
        
        /**
         *  \brief internal method for update
         */
        unsigned int db_update();

        /**
         *  \brief internal method for delete
         */
        unsigned int db_delete();
        
        /**
         *  \brief internal method for check before insert or update
         */
        bool check();



    private:
    
        /**
         * \brief Name of the client
         */
        std::string _name;
};
}

#endif //CLIENT_H_
