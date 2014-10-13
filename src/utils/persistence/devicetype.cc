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

/*! \file devicecype.cc
    \brief Realisation of the class for manipulating with the database 
    objects stored in the table t_bios_device_type.

    \author Alena Chernikava <alenachernikava@eaton.com>
*/  
#include "devicetype.h"
#include <tntdb/connection.h>
#include <tntdb/connect.h>
#include <tntdb/statement.h>
#include <tntdb/value.h>
#include <tntdb/error.h>

namespace utils {

namespace db {

void
DeviceType::
clear_this()
{
    _name = "";
}

DeviceType::
DeviceType(const std::string &url)
    :DataBaseObject(url)
{
    this->clear_this();
}

DeviceType::
DeviceType(const std::string &url, const std::string &name)
    :DataBaseObject(url)
{
    this->clear_this();
    _name = name;
}


DeviceType::
~DeviceType()
{
    //TODO
}

void
DeviceType::
clear()
{
    DataBaseObject::clear();
    this->clear_this();
}

std::string
DeviceType::
getName()
{
    return _name;
}

std::string
DeviceType::
toString()
{
    std::string tmp = DataBaseObject::toString();
    return tmp + ";" + "name="+_name;
}

int DeviceType::selectId(std::string url, std::string name)
{
    tntdb::Connection conn; 
    conn = tntdb::connectCached(url);

    tntdb::Statement st = conn.prepareCached(
        " select "
        " v.id"
        " from"
        " v_bios_device_type v"
        " where v.name = :name"
        );
          
    int tmp = -1;
    
    try{
        tntdb::Value result = st.setString("name", name).selectValue(); 
        result.get(tmp);
    }
    catch (const tntdb::NotFound &e){
        tmp = -1;
    }

    return tmp;
}


unsigned int 
DeviceType::
db_insert()
{
    
    tntdb::Connection conn;  
    conn = tntdb::connectCached(this->getUrl());

    tntdb::Statement st = conn.prepareCached(
        " insert into"
        " v_bios_device_type (id,name)"
        " values (NULL,:name)"
        );
    
    // Insert one row or nothing
    unsigned int n  = st.setString("name", this->getName()).execute();
    
    if ( n == 1 )
    {
        int newId =  conn.lastInsertId();
        this->setId(newId);
    }
    
    // n is 0 or 1
    return n;
}

unsigned int 
DeviceType::
db_delete()
{
    tntdb::Connection conn;  
    conn = tntdb::connectCached(this->getUrl());

    tntdb::Statement st = conn.prepareCached(
        " delete from"
        " v_bios_device_type "
        " where id = :id"
        );
    
    // Delete one row or nothing
    unsigned int n  = st.setInt("id", this->getId()).execute();
    
    // n is 0 or 1
    return n;
}

unsigned int 
DeviceType::
db_update()
{
    
    tntdb::Connection conn;  
    conn = tntdb::connectCached(this->getUrl());

    tntdb::Statement st = conn.prepareCached(
        " update"
        " v_bios_device_type"
        " set name = :name"
        " where id = :id"
        );
    
    // update one row or nothing
    unsigned int n  = st.setString("name", this->getName()).
                         setInt("id",this->getId()).
                         execute();
    
    // n is 0 or 1
    return n;
}

unsigned int 
DeviceType::
selectByName(std::string name)
{
    tntdb::Connection conn; 
    conn = tntdb::connectCached(this->getUrl());
    
    tntdb::Statement st = conn.prepareCached(
        " select"
        " id"
        " from"
        " v_bios_device_type v"
        " where v.name = :name"
        );
    
    /**
     *  Can return one value or nothing, while name is a uniqe
     */
    int n = 0;
    try{
        tntdb::Value result = st.setString("name", name).selectValue();
        
        //id
        int tmp = -1;
        result.get(tmp);
        this->setId(tmp);
        
        //name
        _name = name;
        
        //state
        this->setState(ObjectState::OS_SELECTED);
        
        n = 1; 
    }
    catch (const tntdb::NotFound &e){
        n = 0;
    }
    return n;
}

unsigned int 
DeviceType::
selectById(int id)
{
    tntdb::Connection conn; 
    conn = tntdb::connectCached(this->getUrl()); 
    
    tntdb::Statement st = conn.prepareCached(
        " select"
        " name"
        " from"
        " v_bios_device_type v"
        " where v.id = :id"
        );
    
    /**
     *  Can return one value or nothing
     */
    int n = 0;
    try{
        tntdb::Value result = st.setInt("id", id).selectValue();
        
        //id
        this->setId(id);
        
        //name
        result.get(_name);
    
        //state
        this->setState(ObjectState::OS_SELECTED);
        
        n = 1; 
    }
    catch (const tntdb::NotFound &e){
        n = 0;
    }
    return n;
}

void 
DeviceType::
setName(std::string name)
{
    if ( (_name != name) && (this->getState() != ObjectState::OS_DELETED) )
    {
        switch (this->getState()){
            case ObjectState::OS_SELECTED: 
                this->setState(ObjectState::OS_UPDATED);
            case ObjectState::OS_UPDATED:
            case ObjectState::OS_NEW:
                _name = name;
                break;
            default:
                // TODO log this should never happen
                break;
        }
    }
    //else do nothing
}

bool
DeviceType::
check() const
{
    if ( _name.length() <= this->getNamesLength() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

} // namespace db

}  // end of namespace utils
