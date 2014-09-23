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

/*! \file databasetimeobject.cc
    
    \brief Basic Class for manipulating with database objects that have timestamptcolumn

    This class changes logic of insertion.
    \author Alena Chernikava <alenachernikava@eaton.com>
*/ 
 
#include <string>
#include "databasetimeobject.h"
#include "log.h"
#include <iomanip>
#include <ctime>

namespace utils {

////////////////////////////////////////////////////////////
//////////      DataBaseTimeObject               ///////////
////////////////////////////////////////////////////////////

DataBaseTimeObject::
DataBaseTimeObject(std::string url)
    :DataBaseObject(url)
{
    this->clear();
}

DataBaseTimeObject::
~DataBaseTimeObject()
{
    //TODO
}

std::string 
DataBaseTimeObject::
toString()
{
    time_t t = std::time(nullptr);
    std::string timestr = ctime(&t);
    std::string tmp =   "url="  + this->getUrl()                   + ";" +
                        "id="   + std::to_string(this->getId())    + ";" +
                        "state=" + utils::objectStatetoString(this->getState()) + ";" +
                        "time=" + timestr;
    return tmp;
}
 
unsigned int                // number of row affected
DataBaseTimeObject::
dbsave()
{
    if ( this->getState() == OS_NEW )
    {
        if (this->check())  // if check is true (ok), then proceed
        {
            int n = this->db_insert();
            if ( n > 0 )
                this->setState(OS_INSERTED);
            return n;
        }
        else                // if check is false, then discard an insert
            return 0;
    }
    /* TODO but now updates are not allowed
    else if ( this->getState() == OS_UPDATED )
    {
        if (this->check())
        {
            int n = this->db_update();
            if ( n > 0 )
                this->getState() = OS_SELECTED;
            return n;        
        }
        else
            return 0;
    }*/
    else if ( (this->getState() == OS_SELECTED ) || (this->getState() == OS_INSERTED))
    {
        // do nothing, while it is in actual state
        return 0;
    }
    else    //this->getState() == OS_DELETED
    {
        //do nothing while this object is outofdate
        return 0;
    }

}

time_t
DataBaseTimeObject::
getTimestampt()
{
    return _timestampt;
}

void
DataBaseTimeObject::
clear()
{
    DataBaseObject::clear();
    _timestampt = time(nullptr); 
}

unsigned int
DataBaseTimeObject::
selectTimestampt()
{
    if ( this->getState() == utils::ObjectState::OS_INSERTED )
        return this->db_select_timestampt();
}

void
DataBaseTimeObject::
setTimestampt(time_t timestampt)
{
    _timestampt = timestampt;
}

} // end namespace utils
