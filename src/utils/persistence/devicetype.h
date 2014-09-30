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

/*! \file devicetype.h
    \brief A class for manipulating with the database objects stored 
    in the table t_bios_device_type.

    \author Alena Chernikava <alenachernikava@eaton.com>
*/  
#ifndef DEVICETYPE_H_
#define DEVICETYPE_H_

#include "databaseobject.h"

namespace utils{

/*
 * \brief DeviceType is a class for representing a database entity
 * t_bios_device_type.
 */
class DeviceType: public DataBaseObject
{
    public:

        /**
         * \brief Creates a new object and specifies a connection.
         *
         * Creates a new object for the specified url in the state OS_NEW.
         *
         * \param url - a connection to the database.
         */
        DeviceType(std::string url);
        
        /**
         * \brief Creates a new object with the specified name and 
         * specifies a connection.
         *
         * Creates a new object with the specified name for 
         * the specified url in the state OS_NEW.
         *
         * \param name - a name of the object.
         * \param url  - a connection to the database.
         */
        DeviceType(std::string url, std::string name);

        /**
         * \brief Selects from the DB a device type by its name.
         *
         * Selects from DB device type by name.
         *
         * \return A number of rows selected.
         */
        unsigned int selectByName(std::string name);

        /**
         * \brief Returns device type's name.
         */
        std::string getName();

        /**
         * \brief Sets a new name for the object.
         *
         * If state is OS_DELETED then do nothing.
         * If newname = oldname then do nothing.
         * If state is OS_SELECTED and newname != oldname.
         * than state is changed to OS_UPDATED.
         *
         * \param name - new name of the device type.
         */    
        void setName(std::string name);
        
        /**
         * \brief Selects from DB a device type by ID. Rewrites object.
         *
         * If device type was found:
         * -selects exactly one row.
         * -state changed to OS_SELECTED.
         *
         * If device type was not found:
         * -selects nothing.
         * -everything remains the same.
         *  
         * \param id - id of the row in database.
         *  
         * \return A number of rows selected.
         */
        unsigned int selectById(int id);

        ~DeviceType();
        
        /**
         * \brief Selects an Id by name in the DB specified by url.
         *
         * Selects an Id by name in the DB specified by url.
         * 
         * \param url  - specifies database.
         * \param name - name of the device type.
         * 
         * \return An ID or -1 if nothing was found.
         */
        static int selectId(std::string url, std::string name);

        /**
         * \brief Converts all fields to string and concatenates them.
         *
         * \return Object as string.
         */
        std::string toString();
        
        /**
         * \brief Returns an object to OS_NEW state with initial parameters.
         */
        void clear();

    protected:
        
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
         * \brief Internal method for check before insert or update.
         *
         * Checks the length of the name.
         *
         * \return true if check was successful.
         */
        bool check();

    private:
    
        /**
         * \brief Returns private fields of this object to initial state.
         */
        void clear_this();
        
        /**
         * \brief A name of the deviceType.
         */
        std::string _name;

};  // end of the class

}   // end of namespace utils

#endif //DEVICETYPE_H_
