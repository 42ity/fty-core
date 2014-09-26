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

/*! \file device_discovered.h
    \brief Realisation of the class for manipulating with the database 
    objects stored in the table t_bios_discovered_device

    \author Alena Chernikava <alenachernikava@eaton.com>
*/  
#include "device_discovered.h"
#include "databaseobject.h"     // becauseof datatype
#include "log.h"

#include <tntdb/connection.h>
#include <tntdb/connect.h>
#include <tntdb/result.h>
#include <tntdb/row.h>
#include <tntdb/error.h>

namespace utils{
    
////////////////////////////////////////////////////////////
//////////          DeviceDiscovered                 ///////
////////////////////////////////////////////////////////////

void
DeviceDiscovered::
clear_this()
{
    _name = "unknown";
    _deviceTypeId = -1;
}

void
DeviceDiscovered::
clear()
{
    DataBaseObject::clear();
    this->clear_this();
}

DeviceDiscovered::
DeviceDiscovered(std::string url)
    :DataBaseObject(url)
{
    _clientId = Client::selectId(url,_clientName);
    this->clear_this();
}

DeviceDiscovered::
DeviceDiscovered(std::string url, std::string name)
    :DataBaseObject(url)
{
    this->clear_this();
    _clientId = Client::selectId(url,_clientName);
    _name = name;
}

std::string 
DeviceDiscovered::
toString()
{
    return DataBaseObject::toString() + ";" + "name=" + _name + ";" + "devicetypeid=" + std::to_string(_deviceTypeId);
}

void 
DeviceDiscovered::
setName(std::string name)
{
    if ( ( _name != name ) && ( this->getState() != ObjectState::OS_DELETED ) )
    {
        switch ( this->getState() ){
            case ObjectState::OS_SELECTED: 
                this->setState(ObjectState::OS_UPDATED);
                _name = name;
                break;
            case ObjectState::OS_UPDATED:
            case ObjectState::OS_NEW:
                _name = name;
                break;
            default:
                //log this should never happen
                break;
        }
    }
    //else do nothing
}

void 
DeviceDiscovered::
setDeviceTypeId(int deviceTypeId)
{
    if ( ( _deviceTypeId != deviceTypeId ) && ( this->getState() != ObjectState::OS_DELETED ) )
    {
        switch ( this->getState() ){
            case ObjectState::OS_SELECTED: 
                this->setState(ObjectState::OS_UPDATED);
                _deviceTypeId = deviceTypeId;
                break;
            case ObjectState::OS_UPDATED:
            case ObjectState::OS_NEW:
                _deviceTypeId = deviceTypeId;
                break;
            default:
                //log this should never happen
                break;
        }
    }
    //else do nothing
}


DeviceDiscovered::
~DeviceDiscovered()
{
    //TODO
}

bool
DeviceDiscovered::
check()
{
    if ( _name.length() <= this->getNamesLength() )
        if ( _deviceTypeId > 0 )    // if this field is ok.
            return true;
        else
            return false;
    else
        return false;
}


unsigned int 
DeviceDiscovered::
db_insert()
{
    tntdb::Connection conn;  
    conn = tntdb::connectCached(this->getUrl());

    tntdb::Statement st = conn.prepareCached(
        " insert into"
        " v_bios_discovered_device (id,name,id_device_type)"
        " values (NULL,:name,:iddevicetype)"
        );
    
    // Insert one row or nothing
    unsigned int n  = st.setString("name", this->getName()).
                         setInt("iddevicetype",_deviceTypeId).
                         execute();
    
    if ( n == 1 )
    {
        int newId =  conn.lastInsertId();
        this->setId(newId);
    }
    
    // n is 0 or 1
    return n;
}

unsigned int 
DeviceDiscovered::
db_delete()
{
    tntdb::Connection conn;  
    conn = tntdb::connectCached(this->getUrl());

    tntdb::Statement st = conn.prepareCached(
        " delete from"
        " v_bios_discovered_device "
        " where id = :id"
        );
    
    // Delete one row or nothing
    unsigned int n  = st.setInt("id", this->getId()).execute();
    
    // n is 0 or 1
    return n;
}

unsigned int 
DeviceDiscovered::
db_update()
{
    
    tntdb::Connection conn;  
    conn = tntdb::connectCached(this->getUrl());

    tntdb::Statement st = conn.prepareCached(
        " update"
        " v_bios_discovered_device"
        " set name = :name , id_device_type = :iddevicetype"     //, aaa = :aa
        " where id = :id"
        );
    
    // update one row or nothing
    unsigned int n  = st.setString("name", this->getName()).
                         setInt("id",this->getId()).
                         setInt("iddevicetype",_deviceTypeId).
                         execute();
    
    // n is 0 or 1
    return n;
}

std::vector<DeviceDiscovered> 
DeviceDiscovered::
selectByName(std::string url, std::string name)
{
    tntdb::Connection conn;  
    conn = tntdb::connectCached(url); 

    tntdb::Statement st = conn.prepareCached(
        " select"
        " v.id , v.id_device_type"
        " from"
        " v_bios_discovered_device v"
        " where v.name = :name"
        );

    /**
     *  Can return more than one row, while there could be more than
     *  one discovered device with name "unknown".
     */
    tntdb::Result result = st.setString("name", name).select();

    std::vector<DeviceDiscovered> devicesDiscovered;

    int rsize = result.size();
    if ( rsize > 0 )
    {
        //There are discovered devices with such name
        for ( tntdb::Result::const_iterator it = result.begin();
                it != result.end(); ++it)
        {
            tntdb::Row row = *it;

            DeviceDiscovered* newdeviceDiscovered = new DeviceDiscovered(url);
            
            //id
            int tmp_i = -1;
            row[0].get(tmp_i);
            newdeviceDiscovered->setId(tmp_i);

            //deviceTypeId
            tmp_i = -1;
            row[1].get(tmp_i);
            newdeviceDiscovered->setDeviceTypeId(tmp_i);
            
            //name
            newdeviceDiscovered->setName(name);
                      
            //state
            newdeviceDiscovered->setState(ObjectState::OS_SELECTED);

            devicesDiscovered.push_back(*newdeviceDiscovered);
        }
    }
    //If no rows were selected, return empty vector
    return devicesDiscovered;
}

unsigned int 
DeviceDiscovered::
selectById(int id)
{
    tntdb::Connection conn; 
    conn = tntdb::connectCached(this->getUrl());

    tntdb::Statement st = conn.prepareCached(
        " select"
        " v.name , v.id_device_type"
        " from"
        " v_bios_discovered_device v"
        " where v.id = :id"
        );
    
    /**
     *  Can return one row or nothing
     */
    int n;
    try{
        tntdb::Row row = st.setInt("id", id).selectRow();
        
        //id
        this->setId(id);
        
        //name
        row[0].get(_name);

        //deviceTypeId
        row[1].get(_deviceTypeId);
    
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
DeviceDiscovered::
selectByIp(CIDRAddress ip)
{
    // returns NULL or object
    Ip*  ipObject = Ip::getLastInfo(this->getUrl(), ip.toString());
    if ( ipObject == NULL)
    {
        return -1;
    }
    return this->selectById(ipObject->getDeviceDiscoveredId());
}
     
unsigned int 
DeviceDiscovered::
selectByIp(std::string ip)
{
    // returns NULL or object
    Ip*  ipObject = Ip::getLastInfo(this->getUrl(),ip);
    if ( ipObject == NULL)
    {
        return -1;
    }
    return this->selectById(ipObject->getDeviceDiscoveredId());
}



ClientInfo *
DeviceDiscovered::
selectLastDetailInfo()
{
    if (( this->getState() != ObjectState::OS_NEW ) &&
         ( this->getState() != ObjectState::OS_DELETED ) )
    {

        ClientInfo *newClientInfo = new ClientInfo(this->getUrl());

        int n = newClientInfo->selectLastRecord(this->getDetailedClientId(),this->getId());
        
        if ( n == 1)
            return newClientInfo;
        else if (n > 1 )
        {
            //TODO log this should never happen
            return NULL;
        }
    }
    else
        return NULL;
}


int 
DeviceDiscovered::
getDetailedClientId()
{
    return this->_clientId;
}

std::string 
DeviceDiscovered::getDetailedClientName()
{
    return _clientName;
}

std::string DeviceDiscovered::_clientName = MODULE_ADMIN;

std::string 
DeviceDiscovered::
getName()
{
    return _name;
}

int 
DeviceDiscovered::
getDeviceTypeId()
{
    return _deviceTypeId;
}

} //end of namespace utils
