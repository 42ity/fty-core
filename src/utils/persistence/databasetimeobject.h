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
    \brief Basic Class for manipulating with database objects that have timestamptcolumn

    This class changes logic of insertion.
      
    \author Alena Chernikava <alenachernikava@eaton.com>
*/  
#pragma once    
    
#ifndef DATABASETIMEOBJECT_H_
#define DATABASETIMEOBJECT_H_

#include "databaseobject.h"
#include <string>

namespace utils {
 
/////////////////////////////////////////////////
////////// DataBaseTimeObject ///////////////////
/////////////////////////////////////////////////

/**
 * Represents general methods for manipulating database objects that have timestampt column
 */

class DataBaseTimeObject: public DataBaseObject
{
    public:
        
        /**
         * \brief Save this record to database
         *
         * It does only insert statement and returns a number 
         * of rows affected. 
         * During the inserting an ID would be changed in
         * case of success.
         * After inserting object goes to the state OS_INSERTED.
         * It means, that all fields except timestampt coinsides with database
         */
        unsigned int dbsave();
        
        /**
         * \brief Creates a new object and specifies a connection.
         *
         * Creates a new object for the specified url in state OS_NEW.
         */
        DataBaseTimeObject(std::string url);

        //TODO
        ~DataBaseTimeObject();
        
        /**
         * \brief Returns a string of all fields, incl timestampt 
         */
        std::string toString();

        /**
         * \Brief Returns an object to OS_NEW state with initial parameters
         */
        virtual void clear();

        /* TODO assignment and compasisson
           DataBaseTimeObject ==

           DataBaseTimeObject =

        */

        /**
         * \brief Returns a time of inserting. This field is valid only if the object has state OS_selected.
         */
        time_t getTimestampt();

        /**
         * \brief Selects a timestampt from DB.
         *
         * It selects only if object has state OS_INSERTED.
         * While in OS_NEW - no,
         *       in OS_DELETED non valid
         *       in OS_UPDATED this state isn't use here.
         *       in OS_SELETED is already selected
         */
        unsigned int selectTimestampt();

    protected:
        
        virtual unsigned int db_insert(){return 1;};
        
        /**
         *  \brief internal method for update
         * TODO but update is not allowed
         */
        unsigned int db_update(){return 0;};

        /**
         *  \brief internal method for delete
         * Must be rewritten in every child-class
         */
        virtual unsigned int db_delete(){return 1;};
        
        /**
         *  \brief internal method for check before insert or update
         * Must be rewritten in every child-class
         */
        virtual bool check(){return true;};

        /**
         * \brief Selects a timestempt for stored ID.
         *
         * Internal method.
         * Mustbe rewritten in every child-class
         */
        virtual unsigned int db_select_timestampt(){};

        /**
         * Is used for filling the object. It is not designed for 
         * using outside the child classes.
         */
        void setTimestampt(time_t timestampt);
        
    private:

        /**
         * \brief Time of inserting. Is filled automatically with NOW() statement of database.
         *
         * User cannot modify it manually.
         */
        time_t _timestampt;

};


} // namespace utils

#endif // DATABASETIMEOBJECT_H_
