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

/*! \file clientinfo.cc
    
    \brief Realisation of the class for databaseobject t_bios_client_info
    
    \author Alena Chernikava <alenachernikava@eaton.com>
*/ 
 

#include "clientinfo.h"
#include <tntdb/connection.h>
#include <tntdb/connect.h>
#include <tntdb/result.h>
#include <tntdb/row.h>
#include "client.h"

namespace utils {
    
////////////////////////////////////////////////////
///////////// ClientInfo ///////////////////////////
////////////////////////////////////////////////////

void
ClientInfo::
clear_this()
{
    _clientName = "";
    _clientId = -1;
    _discoveredDeviceId = -1;
    _blobData = "";
}

void
ClientInfo::
clear()
{
    DataBaseTimeObject::clear();
    this->clear_this();
}  

ClientInfo::
ClientInfo(std::string url)
    :DataBaseTimeObject(url)
{
    this->clear_this();
}

ClientInfo::
ClientInfo(std::string url, std::string clientName)
    :DataBaseTimeObject(url)
{
    this->clear_this();
    _clientName = clientName;
    _clientId = Client::selectId(url, clientName);
}

ClientInfo::
ClientInfo(std::string url,  int clientId)
    :DataBaseTimeObject(url)
{
    this->clear_this();
    this->setClientId(clientId);
}

std::string 
ClientInfo::
toString()
{
    return DataBaseTimeObject::toString()                                      + ";" +
           "clientId="                   + std::to_string(_clientId)           + ";" +
           "clientName="                 + _clientName                         + ";" +
           "deviceId="                   + std::to_string(_discoveredDeviceId) + ";" +
           "blobdata="                   + _blobData                           ;
}

unsigned int ClientInfo::selectLastRecord()
{
    return  this->selectLastRecord(this->_clientId,this->_discoveredDeviceId);
}

bool
ClientInfo::
check()
{
    if ( _clientId > 0 )    //_clientId is NOTNULL
        return true;
    else
        return false;
}

unsigned int 
ClientInfo::
db_insert()
{
    
    tntdb::Connection conn;  
    conn = tntdb::connectCached(this->getUrl());

    tntdb::Statement st = conn.prepareCached(
        " insert into"
        " v_bios_client_info (id, id_client, id_discovered_device, ext, timestamp)"
        " values (NULL,:idclient,:iddiscovereddevice, :ext, NOW())"
        );
    
    // Insert one row or nothing
    st.setInt("idclient", _clientId).
       setString("ext",_blobData);
 
    unsigned int n = 0;
    if ( _discoveredDeviceId == -1 )
        n = st.setNull("iddiscovereddevice").execute();
    else
        n = st.setInt("iddiscovereddevice",_discoveredDeviceId).execute();

    if ( n == 1 )
    {
        int newId =  conn.lastInsertId();
        this->setId(newId);
    }
    
    //We don't want get back to application level time of inserting.
    //In case of need use after inserting a method selectTimestamp.
 
    // n is 0 or 1
    return n;
}

unsigned int 
ClientInfo::
db_delete()
{
    tntdb::Connection conn;  
    conn = tntdb::connectCached(this->getUrl());

    tntdb::Statement st = conn.prepareCached(
        " delete from"
        " v_bios_client_info "
        " where id = :id"
        );
    
    // Delete one row or nothing
    unsigned int n  = st.setInt("id", this->getId()).execute();
    
    // n is 0 or 1
    return n;
}

unsigned int 
ClientInfo::
db_update()
{
    return 0;
    //TODO but now we don't want to modify already existing data.    
}

// if any changes would be made here check if the same need to do in
// ClientInfo::selectLastRecord(std::string clientName, int discoveredDeviceId)
unsigned int 
ClientInfo::
selectLastRecord(int clientId, int discoveredDeviceId)
{
    tntdb::Connection conn; 
    conn = tntdb::connectCached(this->getUrl()); 

    tntdb::Statement st = conn.prepareCached(
        " select "
        " v.id, v.datum, v.info, v.name"
        " from"
        " v_bios_client_info_last v"
        " where v.id_discovered_device = :id_devicediscovered and v.id_client = :id_client"
        );
    
    //   Should return one row or nothing.
    tntdb::Result result = st.setInt("id_deviceidscovered", discoveredDeviceId).
                              setInt("id_client", clientId).
                              select();
    int rsize = result.size();
    if ( rsize == 1)
    {
        tntdb::Result::const_iterator it = result.begin();
        tntdb::Row row = *it;

        //id
        int tmp = -1;
        row[0].get(tmp);
        this->setId(tmp);
    
        //timestamp
        time_t tmp_t = time(NULL);  // if get-method got NULL, than it doesn't modify variable. 
                                       // So need to define initial value.
                                       // but it should never happen, while this column must be NOT NULL
        bool isNotNull = row[1].get(tmp_t);
        if (isNotNull)
            this->setTimestamp(tmp_t);
        else
        {
            //TODO
            //log THIS SHOULD NEVER HAPPEN
        }

        //blobData
        row[2].get(_blobData);
    
        //clientId
        _clientId = clientId;
        
        //clientName
        row[3].get(_clientName);

        //discoveredDeviceId
        _discoveredDeviceId = discoveredDeviceId;
                        
        this->setState(ObjectState::OS_SELECTED);
    }
    else if (rsize > 1)
    {
        //log this should never hapen
    }
    return rsize;
}

unsigned int
ClientInfo::
db_select_timestamp()
{
    tntdb::Connection conn; 
    conn = tntdb::connectCached(this->getUrl());
    
    tntdb::Statement st = conn.prepareCached(
        " select"
        " v.timestamp"
        " from"
        " v_bios_client_info v"
        " where v.id = :id"
        );
    
    /**
     *  Can return one row or nothing
     */
    int n;
    try{
        tntdb::Row row = st.setInt("id", this->getId()).selectRow();
          
        //timestamp
        time_t tmp_t = time(NULL);  // if get-method got NULL, than it doesn't modify variable. 
                                    //So need to define initial value.
                                    // but it should never happen, while this column must be NOT NULL
        bool isNotNull = row[0].get(tmp_t);
        if (isNotNull)
            this->setTimestamp(tmp_t);
        else
        {
            //TODO
            //log THIS SHOULD NEVER HAPPEN
        }
        
        //state
        this->setState(ObjectState::OS_SELECTED);
        
        n = 1; 
    }
    catch (const tntdb::NotFound &e){
        n = 0;
    }
    return n;
}

// if any changes would be made here check if the same need to do in
// ClientInfo::selectLastRecord(int clientId, int discoveredDeviceId)
unsigned int ClientInfo::selectLastRecord(std::string clientName, int discoveredDeviceId)
{
    tntdb::Connection conn; 
    conn = tntdb::connectCached(this->getUrl()); 

    tntdb::Statement st = conn.prepareCached(
        " select "
        " v.id, v.datum, v.info, v.id_client"
        " from"
        " v_bios_client_info_last v"
        " where v.id_discovered_device = :id_devicediscovered and v.client_name = :clientname"
        );
    
    //   Should return one row or nothing.
    tntdb::Result result = st.setInt("id_devicediscovered", discoveredDeviceId).
                              setString("clientname", clientName).
                              select();
    int rsize = result.size();
    if ( rsize == 1)
    {
        tntdb::Result::const_iterator it = result.begin();
        tntdb::Row row = *it;

        //id
        int tmp = -1 ;
        row[0].get(tmp);
        this->setId(tmp);
    
        //timestamp
        time_t tmp_t = time(NULL);  // if get-method got NULL, than it doesn't modify variable. 
                                       // So need to define initial value.
                                       // but it should never happen, while this column must be NOT NULL
        bool isNotNull = row[1].get(tmp_t);
        if (isNotNull)
            this->setTimestamp(tmp_t);
        else
        {
            //TODO
            //log THIS SHOULD NEVER HAPPEN
        }
        
        //blobData
        row[2].get(_blobData);
    
        //clientName
        _clientName = clientName;
        
        //clientId
        row[3].get(_clientId);

        //discoveredDeviceId
        _discoveredDeviceId = discoveredDeviceId;
                        
        this->setState(ObjectState::OS_SELECTED);
    }
    else if (rsize > 1)
    {
        //log this should never hapen
    }
    return rsize;
}

 int
ClientInfo::
getClientId()
{
    return _clientId;
}

 int
ClientInfo::
getDeviceDiscoveredId()
{
    return _discoveredDeviceId;
}

std::string
ClientInfo::
getBlobData()
{
    return _blobData;
}

std::string
ClientInfo::
getClientName()
{
    return _clientName;
}

void
ClientInfo::
setClientId(int clientId)
{
    if ( (_clientId != clientId) && (this->getState() != ObjectState::OS_DELETED) )
    {
        switch (this->getState()){
            case ObjectState::OS_SELECTED:
            case objectState::OS_INSERTED:
                this->setState(ObjectState::OS_UPDATED);
                if (this->selectClientName(clientId))
                    _clientId = clientId;
                break;
            case ObjectState::OS_UPDATED:
            case ObjectState::OS_NEW:
                if (this->selectClientName(clientId))
                    _clientId = clientId;
                break;
            default:
                // TODO log this should never happen
                break;
        }
    }
}

ClientInfo::
~ClientInfo()
{
    //TODO
}

void
ClientInfo::
setBlobData(std::string blobData)
{
    if ( (_blobData != blobData) && (this->getState() != ObjectState::OS_DELETED) )
    {
        switch (this->getState()){
            case ObjectState::OS_SELECTED:
            case objectState::OS_INSERTED:
                this->setState(ObjectState::OS_UPDATED);
            case ObjectState::OS_UPDATED:
            case ObjectState::OS_NEW:
                _blobData = blobData;
                break;
            default:
                // TODO log this should never happen
                break;
        }
    }
    //else do nothing
}

void
ClientInfo::
setDeviceDiscoveredId(int discoveredDeviceId)
{
    if ( (_discoveredDeviceId != discoveredDeviceId) && (this->getState() != ObjectState::OS_DELETED) )
    {
        switch (this->getState()){
            case ObjectState::OS_SELECTED:
            case objectState::OS_INSERTED:
                this->setState(ObjectState::OS_UPDATED);
            case ObjectState::OS_UPDATED:
            case ObjectState::OS_NEW:
                _discoveredDeviceId = discoveredDeviceId;
                break;
            default:
                // TODO log this should never happen
                break;
        }
    }
    //else do nothing
}

bool
ClientInfo::
selectClientName(int clientId)
{
    Client client = Client(this->getUrl());
    client.selectById(clientId);
    if (client.getState() == ObjectState::OS_SELECTED)
    {
        _clientName = client.getName();
        return true;
    }
    else
        return false;
}

unsigned int
ClientInfo::
selectById(int id)
{
    tntdb::Connection conn; 
    conn = tntdb::connectCached(this->getUrl()); 

    tntdb::Statement st = conn.prepareCached(
        " select "
        " v.timestamp, v.ext, v.id_client , v.id_discovered_device"
        " from"
        " v_bios_client_info v"
        " where v.id = :id"
        );

    //   Can return one row or nothing
    int n = 0;
    try{ 
        tntdb::Row row = st.setInt("id", id).selectRow();
          
        //timestamp
        time_t tmp_t = time(nullptr);  // if get-method got NULL, than it doesn't modify variable. 
                                       // So need to define initial value.
                                       // but it should never happen, while this column must be NOT NULL
        bool isNotNull = row[0].get(tmp_t);
        if (isNotNull)
            this->setTimestamp(tmp_t);
        else
        {
            //TODO
            //log THIS SHOULD NEVER HAPPEN
        }
                        
        //id
        this->setId(id);
    
        //blobData
        row[1].get(_blobData);
    
        //clientId
        int tmp_i = -1;
        row[2].get(tmp_i);
        this->setClientId(tmp_i);
        
        //discoveredDeviceId
        tmp_i = -1;
        row[3].get(tmp_i);
        _discoveredDeviceId = tmp_i;

        //state
        this->setState(ObjectState::OS_SELECTED);
        
        n = 1; 
    }
    catch (const tntdb::NotFound &e){
        n = 0;
    }
    return n;
}

} //end of namespace utils
