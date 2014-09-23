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

/*! \file databaseobject.h
    \brief Basic Class for manipulating with database objects

    This class consists of basic fields and methods, that are common
    for all database objects.

    and some enums.
      
    \author Alena Chernikava <alenachernikava@eaton.com>
*/  
#pragma once    
    
#ifndef DATABASEOBJECT_H_
#define DATABASEOBJECT_H_

#include <string>

namespace utils {
 
/**
 * represents the relation of the object to the database
 */
enum ObjectState{
    OS_SELECTED,    //Object fully coresponds with the row
    OS_NEW,         //Object is new, there is no record in DB
    OS_UPDATED,     //Object is in DB, but here is a modified version
    OS_DELETED,     //Object was deleted from DB
    OS_INSERTED     //Object was successfully inserted to DB, but at the application level field "date" is out of date
};

/** 
 * represents a way of selection by date
 */
enum dateType{
    DT_BEFORE_DATE,
    DT_BEFORE_DATE_INCL,
    DT_AFTER_DATE,
    DT_AFTER_DATE_INCL,
    DT_BETWEEN
};

/**
 * helper for the function toString for objects
 */
std::string objectStatetoString(ObjectState objectstate);

/////////////////////////////////////////////////
////////// DataBaseObject ///////////////////////
/////////////////////////////////////////////////

/**
 * represents general methods for database objects
 */

class DataBaseObject
{
    public:
        
        /**
         * \brief Save this record to database
         *
         * It does insert or update statement and returns a number 
         * of rows affected. During the inserting an ID would be change in
         * case of success.
         */
        unsigned int dbsave();
        
        /**
         * \brief Delete a record from database by ID
         *
         * it Changes status to OS_DELETED
         */
        unsigned int dbdelete();
        
        /**
         * \brief Creates a new object and specifies a connection.
         *
         * Creates a new object for the specified url in state OS_NEW.
         */
        DataBaseObject(std::string url);

        //TODO
        ~DataBaseObject();
        
        /**
         * \brief Get a value of state
         */
        ObjectState getState();

        /**
         * \brief Get a value of ID
         */
        int getId();

        /**
         * \brief Get a value of url
         */
        std::string getUrl();

        /**
         * \brief Returns a string of all fields 
         */
        std::string toString();

        /**
         * \Brief Returns an object to OS_NEW state with initial parameters
         * Must be rewritten in every child-class
         */
        virtual void clear();

        /**
         * \brief Get a maximum length of all fields "name" in database.
         */
        static int getNamesLength();
       
        /**
         * \brief A prototype method. Selects object by Id.
         * 
         * Must be rewritten in every child-class
         */
        virtual unsigned int selectById(unsigned int id){return 1;};
        
        /**
         * \brief Reloads this object by its id from the DB
         *
         * In case of state OS_NEW/OS_DELETED there is nothing to reload
         */
        unsigned int reload();
         
        /* TODO assignment and compasisson
        DataBaseObject ==

        DataBaseObject =

        */
    protected:
        
        /**
         * \brief Set a new state for the object.
         *
         * For Class internal use only
         */    
        void setState(ObjectState state);
        
        /**
         * Is used for filling the object. It is not desiened for 
         * using outside the child classes.
         */
        void setId(int id);

        /**
         *  \brief internal method for insert 
         * Must be rewritten in every child-class
         */
        virtual unsigned int db_insert(){return 1;};
        
        /**
         *  \brief internal method for update
         * Must be rewritten in every child-class
         */
        virtual unsigned int db_update(){return 1;};

        /**
         *  \brief internal method for delete
         * Must be rewritten in every child-class
         */
        virtual unsigned int db_delete(){return 1;};
        
        /**
         *  \brief internal method for check before insert or update
         * Must be rewritten in every child-class
         */
        virtual bool check(){ return true;};


    private:
        
        /**
         * \brief A state of the object with the relation to the DB
         */    
        ObjectState _state;
        
        /**
         * \brief An url that specifies a connection to the DB
         */
        std::string _url;
    
        /**
         * \brief Id of the row 
         * 
         * Id is valid only if its state is not OS_NEW
         */
        int _id;
        
        /**
         * \brief A maximum length of all fields "name" in database.
         *
         * TODO is it 25?
         * while if length of string is more than length of field it 
         * would be cutted without any error
         */
        static int _names_length;
};


} // namespace utils

#endif // DATABASEOBJECT_H_
