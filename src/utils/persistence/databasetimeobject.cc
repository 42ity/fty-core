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
    
    \brief A basic Class for manipulating with database objects that have timestamp column

    This class changes logic of insertion.
    
    \author Alena Chernikava <alenachernikava@eaton.com>
*/ 
 
#include <string>
#include "databasetimeobject.h"
#include "log.h"
#include <iomanip>
#include <ctime>
namespace utils {

namespace db {

time_t convertToCTime(const tntdb::Datetime &datetime)
{
    struct tm timeinfo;

    timeinfo.tm_year = datetime.getYear()-1900;
    timeinfo.tm_mon  = datetime.getMonth()-1;
    timeinfo.tm_mday = datetime.getDay();
    timeinfo.tm_hour = datetime.getHour();
    timeinfo.tm_min  = datetime.getMinute(); 
    timeinfo.tm_sec  = datetime.getSecond();
    timeinfo.tm_isdst = true;
    
    return mktime ( &timeinfo );
}

DataBaseTimeObject::
DataBaseTimeObject(const std::string &url)
    :DataBaseObject(url)
{
    this->clear_this();
}

void
DataBaseTimeObject::
clear_this()
{
    _timestamp = time(NULL); 
}

DataBaseTimeObject::
~DataBaseTimeObject()
{
    //TODO
}

std::string 
DataBaseTimeObject::
toString() const
{
    std::string timestr = ctime(&_timestamp);
    std::string tmp =   DataBaseObject::toString() + ";" +
                        "time=" + timestr;
    return tmp;
}
 
unsigned int                
DataBaseTimeObject::
dbsave()
{
    if ( this->getState() == ObjectState::OS_NEW )
    {
        if (this->check())  // if check is true (ok), then proceed
        {
            int n = this->db_insert();
            if ( n > 0 )
                this->setState(ObjectState::OS_INSERTED);
            return n;
        }
        else                // if check is false, then discard an insert
            return 0;
    }
    /* TODO but now updates are not allowed
    else if ( this->getState() == ObjectState::OS_UPDATED )
    {
        if (this->check())
        {
            int n = this->db_update();
            if ( n > 0 )
                this->getState() = ObjectState::OS_SELECTED;
            return n;        
        }
        else
            return 0;
    }*/
    else //if ( (this->getState() == ObjectState::OS_SELECTED ) || ( this->getState() == ObjectState::OS_INSERTED) 
        //|| (this->getState() == ObjectState::OS_DELETED) )
    {
        // do nothing
        return 0;
    }
}

time_t
DataBaseTimeObject::
getTimestamp()
{
    return _timestamp;
}

void
DataBaseTimeObject::
clear()
{
    DataBaseObject::clear();
    this->clear_this();
}

unsigned int
DataBaseTimeObject::
selectTimestamp()
{
    if ( this->getState() == ObjectState::OS_INSERTED )
        return this->db_select_timestamp();
    return 0;
}

void
DataBaseTimeObject::
setTimestamp(time_t timestamp)
{
    _timestamp = timestamp;
}

} // namespace db

} // end namespace utils

