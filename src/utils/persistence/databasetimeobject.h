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

/*! \file databasetimeobject.h
    \brief A basic class for manipulating with database objects that have timestamp column.

    This class changes logic of insertion.
      
    \author Alena Chernikava <alenachernikava@eaton.com>
*/  
    
#ifndef DATABASETIMEOBJECT_H_
#define DATABASETIMEOBJECT_H_

#include "databaseobject.h"
#include <string>

namespace utils {
 
/////////////////////////////////////////////////
////////// DataBaseTimeObject ///////////////////
/////////////////////////////////////////////////

/**
 * \brief Represents general methods for manipulating database objects that have timestamp column.
 */

class DataBaseTimeObject: public DataBaseObject
{
    public:
        
        /**
         * \brief Saves this object to database.
         *
         * It does only insert statement.
         *
         * In case od success an Id would be changed and state would be OS_INSERTED.
         * It means, that all fields except timestamp coinsides with database.
         * 
         * \return A number of rows affected. 
         */
        unsigned int dbsave();
        
        /**
         * \brief Creates a new object and specifies a connection.
         *
         * Creates a new object for the specified url in state OS_NEW.
         * 
         * \param url - connection to the database
         */
        DataBaseTimeObject(std::string url);

        ~DataBaseTimeObject();
        
        /**
         * \brief Converts all fields to string and concatenates them.
         *
         * \return Object as string.
         */
        std::string toString();

        /**
         * \brief Returns an object to OS_NEW state with initial parameters.
         * 
         * Must be rewritten in every child-class.
         */
        virtual void clear();

        /**
         * \brief Gets a timestamp.
         *
         * This field is valid only if the object has state OS_SELECTED.
         *
         * \return A timestamp.
         */
        time_t getTimestamp();

        /**
         * \brief Selects a timestamp from DB.
         *
         * It selects only if object has state OS_INSERTED.
         * While in OS_NEW - nothing to select,
         *       in OS_DELETED object is already not valid
         *       in OS_UPDATED this state is invalid for historical data.
         *       in OS_SELETED is already selected.
         */
        unsigned int selectTimestamp();

    protected:
        
        /**
         * \brief Internal method for insert.
         *
         * Must be rewritten in every child-class.
         *
         * \return A number of rows affected.
         */
        virtual unsigned int db_insert(){return 1;};
        
        /**
         * \brief Internal method for update.
         *
         * \return A number of rows affected.
         */
        unsigned int db_update(){return 0;};

        /**
         * \brief Internal method for delete.
         *
         * Must be rewritten in every child-class.
         *
         * \return A number of rows affected.
         */
        virtual unsigned int db_delete(){return 1;};
        
        /**
         * \brief Internal method for check before insert or update.
         * 
         * Must be rewritten in every child-class.
         *
         * \return true if check was successful.
         */
        virtual bool check(){return true;};

        /**
         * \brief Selects a timestamp for stored Id.
         *
         * Internal method.
         *
         * Mustbe rewritten in every child-class.
         * 
         * \return true if check was successful.
         */
        virtual unsigned int db_select_timestamp(){};

        /**
         * \brief Sets a new Timestamp for the object.
         *
         * It is used for filling the object. It is not desiened for 
         * using outside the child classes.
         *
         * \param A new timestamp.
         */
        void setTimestamp(time_t timestamp);
        
    private:
        
        /**
         * \brief Returns private fields of this object to initial state.
         */
        void clear_this();

        /**
         * \brief Time of inserting. Is filled automatically with NOW() statement of database.
         *
         * User cannot modify it manually.
         */
        time_t _timestamp;

}; // end of the class


}  // end of namespace utils

#endif // DATABASETIMEOBJECT_H_
