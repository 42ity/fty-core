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
    \brief The basic class for manipulating with database objects

    This class consists of basic fields and methods, that are common
    for all database objects.

    Here are introduled two enums. One for the state of the objects and one 
    for manipulating with the history.
      
    \author Alena Chernikava <alenachernikava@eaton.com>
*/

#ifndef UTILS_PERSISTENCE_DATABASEOBJECT_H_
#define UTILS_PERSISTENCE_DATABASEOBJECT_H_

#include <string>

namespace utils {

namespace db {
 
/**
 * \brief Represents the relation of the object to the database
 */
enum class ObjectState : char{
    OS_SELECTED = 's',  //Object fully coresponds with the row
    OS_NEW      = 'n',  //Object is new, there is no record in DB
    OS_UPDATED  = 'u',  //Object is in DB, but here is a modified version
    OS_DELETED  = 'd',  //Object was deleted from DB
    OS_INSERTED = 'i'   //Object was successfully inserted to DB, but at the 
                        //application level field "timestamp" is out of date
};

/** 
 * \brief Represents a way of selection by date
 */
enum class dateType : char{
    DT_BEFORE_DATE      = 'b', //Selection of the history before specified date
    DT_BEFORE_DATE_INCL = 'B', //Selection of the history before specified date
                               //inclusive
    DT_AFTER_DATE       = 'a', //Selection of the history after specified date
    DT_AFTER_DATE_INCL  = 'A', //Selection of the history after specified date 
                               //inclusive
    DT_BETWEEN          = 'm'  //Selection of the history between specified 
                               //dates
};

/**
 * helper for the function toString for objects
 */
std::string objectStatetoString(ObjectState objectstate);

/////////////////////////////////////////////////
////////// DataBaseObject ///////////////////////
/////////////////////////////////////////////////

/**
 * \Brief This class represents general methods and fields that are common 
 * for all for database objects.
 *
 * But this behaviour could be changed in child classed in case of need.
 */
class DataBaseObject
{
    public:
        
        /**
         * \brief Saves this object to database.
         *
         * It does insert or update statement.
         * In case of successful insert:
         *      - an Id of the object would be changed to match 
         *        the actual state.
         * In case of successful update/insert:
         *      - a state would be chaned to OS_SELECTED
         * 
         * \return A number of rows affected.
         */
        unsigned int dbsave();
        
        /**
         * \brief Deletes this object from database by Id.
         *
         * In case of success object status would be changed to OS_DELETED.
         *
         * \return A number of rows affected.
         */
        unsigned int dbdelete();
        
                
        /**
         * \brief Gets a value of object's state.
         *
         * \return Object's state.
         */
        ObjectState getState() const;

        /**
         * \brief Gets a value of object's ID.
         *
         * Id is validonly in OS_SELECTED and OS_UPDATED states.
         *
         * \return Object's Id.
         */
        int getId() const;

        /**
         * \brief Gets a value of object's url.
         * 
         * \return Object's url.
         */
        std::string getUrl() const;

        /**
         * \brief Converts all fields to string and concatenates them.
         *
         * Must be rewritten in every child-class.
         * 
         * \return Object as string.
         */
        virtual std::string toString() const;

        /**
         * \brief Returns an object to OS_NEW state with initial parameters.
         * 
         * Must be rewritten in every child-class.
         */
        virtual void clear();

        /**
         * \brief Gets a maximum length of all fields "name" in database.
         *
         * \return A maximum length of "name"-fields in database.
         */
        static unsigned int getNamesLength();
       
        /**
         * \brief A prototype method. Selects object by Id.
         * 
         * Must be rewritten in every child-class.
         *
         * \return A number of rows affected.
         */
        virtual unsigned int selectById(int id) = 0;
        
        /**
         * \brief Reloads this object by its id from the DB.
         *
         * In case of state OS_NEW/OS_DELETED/OS_SELECTED there is nothing 
         * to reload.
         *
         * \return A number of rows affected.
         */
        unsigned int reload();
        
        /**
         * \brief Deletes the object with specified ID from the DB
         *
         * \return A number of rows deleted.
         */
        unsigned int deleteById(int id);
 
        /* TODO assignment and compasisson
        DataBaseObject ==

        DataBaseObject =

        */
    protected:
       
       /**
         * \brief Creates a new object and specifies a connection.
         *
         * Creates a new object for the specified url in state OS_NEW.
         * 
         * \param url - connection to the database
         */
        DataBaseObject(const std::string &url);

        //TODO
        ~DataBaseObject();
 
        /**
         * \brief Sets a new state for the object.
         *
         * For Class internal use only.
         *
         * \param A new state.
         */    
        void setState(ObjectState state);
        
        /**
         * \brief Sets a new ID for the object.
         *
         * It is used for filling the object. It is not desiened for 
         * using outside the child classes.
         *
         * \param A new Id.
         */
        void setId(int id);

        /**
         * \brief Internal method for insert.
         *
         * Must be rewritten in every child-class.
         *
         * \return A number of rows affected.
         */
        virtual unsigned int db_insert() = 0;
        
        /**
         * \brief Internal method for update.
         *
         * Must be rewritten in every child-class.
         *
         * \return A number of rows affected.
         */
        virtual unsigned int db_update() = 0;

        /**
         * \brief Internal method for delete.
         *
         * Must be rewritten in every child-class.
         *
         * \return A number of rows affected.
         */
        virtual unsigned int db_delete() = 0;
        
        /**
         * \brief Internal method for check before insert or update.
         * 
         * Must be rewritten in every child-class.
         *
         * \return true if check was successful.
         */
        virtual bool check() const {return true;};


    private:
        
        /**
         * \brief A state of the object with the relation to the DB.
         */    
        ObjectState _state;
        
        /**
         * \brief An url that specifies a connection to the DB.
         */
        std::string _url;
    
        /**
         * \brief Id of the row.
         * 
         * Id is valid only if its state is not OS_NEW/OS_DELETED.
         */
        int _id;
        
        /**
         * \brief A maximum length of all fields "name" in database.
         *
         * TODO is it 25?
         */
        static int _names_length;

}; // end of the class

} // namespace db

}  // end of namespace utils

#endif // DATABASEOBJECT_H_
