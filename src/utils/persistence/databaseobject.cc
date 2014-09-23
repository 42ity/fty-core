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

/*! \file databaseobject.cc
    
    \brief Realisation of the class for basic databaseobject.
    \author Alena Chernikava <alenachernikava@eaton.com>
*/ 
 
#include <string>
#include <iostream>
#include "databaseobject.h"
#include "log.h"

namespace utils {

////////////////////////////////////////////////////////////
//////////      DataBaseObject               ///////////////
////////////////////////////////////////////////////////////

// is used only for internal porposes. 
void 
DataBaseObject::
setId(int id)
{
    _id = id;
}

void 
DataBaseObject::
setState(ObjectState state)
{
    _state = state;
    if ( _state == OS_NEW )
        _id = -1;
}

DataBaseObject::
DataBaseObject(std::string url)
{
    // TODO how to check if the connection is ok?
    _url = url;
    this->clear();
}

ObjectState 
DataBaseObject::
getState()
{
    return _state;
}

int 
DataBaseObject::
getId()
{
    if ( ( ( _state == OS_NEW ) || (_state == OS_DELETED ))&& ( _id != -1 ) )
    {
            //TODO log(this should never happen)
    }
    return _id;
}

std::string 
DataBaseObject::
getUrl()
{
    return _url;
}

DataBaseObject::
~DataBaseObject()
{
    //TODO
}

std::string 
DataBaseObject::
toString()
{
    std::string tmp =   "url="  + _url                   + ";" +
                        "id="   + std::to_string(_id)    + ";" +
                        "state=" + utils::objectStatetoString(_state) ;
    return tmp; 
}

unsigned int                // number of row affected
DataBaseObject::
dbsave()
{
    if ( _state == OS_NEW )
    {
        if (this->check())  // if check is true (ok), then proceed
        {
            int n = this->db_insert();
            if ( n > 0 )
                _state = OS_SELECTED;
            return n;
        }
        else                // if check is false, then discard an insert
            return 0;
    }
    else if ( _state == OS_UPDATED )    //a DataBaseObject never rich this state, only its child could
    {
        if (this->check())
        {
            int n = this->db_update();
            if ( n > 0 )
                _state = OS_SELECTED;
            return n;        
        }
        else
            return 0;
    }
    else if (_state == OS_SELECTED )
    {
        // do nothing
        return 0;
    }
    else    //_state == OS_DELETED or OS_INSERTED
    {
        //do nothing
        return 0;
        //TODO
    }

}

// delete object by its id
unsigned int 
DataBaseObject::
dbdelete()
{
    if ( _state == OS_NEW )
    {   // no id is known
        //do nothing
        return 0;
    }
    else if ( ( _state == OS_UPDATED ) || ( _state == OS_SELECTED ) || (_state == OS_INSERTED))
    {
        int n = this->db_delete();
        if ( n > 0 )
        {
            _state = OS_DELETED;
            _id = -1;               // make id non valid
            //TODO add some stuff
        }
        return n;        
    }
    else if (_state == OS_DELETED )
    {
        // do nothing, the object is bad;
        return 0;
    }
}

int DataBaseObject::_names_length = 25;

int
DataBaseObject::
getNamesLength()
{
    return _names_length;
}

//where I need to put it TODO
std::string 
objectStatetoString(ObjectState objectstate)
{
    switch (objectstate){
        case OS_NEW:        return "osnew";
        case OS_SELECTED:   return "osselected";
        case OS_UPDATED:    return "osupdated";
        case OS_DELETED:    return "osdeleted";
        case OS_INSERTED:   return "osinserted";
        default:            return "osunknown";
    }
}

unsigned int 
DataBaseObject::
reload()
{
    if ((_state != ObjectState::OS_NEW) && (_state != ObjectState::OS_DELETED))
    {
        return this->selectById(_id);
    }
    else
        return 0;
}

void
DataBaseObject::
clear()
{
    _state = OS_NEW;
    _id = -1;
}

} // end namespace utils
