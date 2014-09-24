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
clear()
{
    DataBaseTimeObject::clear();
    _clientName = "";
    _clientId = -1;
    _deviceDiscoveredId = -1;
    _blobData = "";
}  

ClientInfo::
ClientInfo(std::string url)
    :DataBaseTimeObject(url)
{
    this->clear();
}

ClientInfo::
ClientInfo(std::string url, std::string clientName)
    :DataBaseTimeObject(url)
{
    this->clear();
    _clientName = clientName;
    _clientId = Client::selectId(url, clientName);
}

ClientInfo::
ClientInfo(std::string url, unsigned int clientId)
    :DataBaseTimeObject(url)
{
    this->clear();
    this->setClientId(clientId);
}

std::string 
ClientInfo::
toString()
{
    return DataBaseTimeObject::toString() + ";" +
                            "clientId=" + std::to_string(_clientId)           + ";" +
                            "clientName"+ _clientName         + ";" +
                            "deviceId=" + std::to_string(_deviceDiscoveredId) + ";" +
                            "blobdata=" + _blobData;
}

unsigned int ClientInfo::selectLastRecord()
{
    return  this->selectLastRecord(this->_clientId,this->_deviceDiscoveredId);
}

unsigned int 
ClientInfo::
db_insert()
{
    
    tntdb::Connection conn;  
    conn = tntdb::connectCached(this->getUrl());

    tntdb::Statement st = conn.prepareCached(
        " insert into"
        " v_bios_client_info (id, id_client, id_discovered_device, ext, timestampt)"
        " values (NULL,:idclient,:iddiscovereddevice, :ext, NOW())"
        );
    
    // Insert one row or nothing
    unsigned int n  = st.setInt("idclient", _clientId).
                         setInt("iddiscovereddevice",_deviceDiscoveredId).
                         setString("ext",_blobData).
                         execute();
    
    if ( n == 1 )
    {
        int newId =  conn.lastInsertId();
        this->setId(newId);
    }
    
    //We don't want get back to application level time of inserting.
    //In case of need use after inserting a method selectTimestampt.
 
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
    //TODO but now we don't want to modify already existing data.    
}

// if any changes would be made here check if the same need to do in
// ClientInfo::selectLastRecord(std::string clientName, int deviceDiscoveredId)
unsigned int 
ClientInfo::
selectLastRecord(int clientId, int deviceDiscoveredId)
{
    tntdb::Connection conn; 
    conn = tntdb::connectCached(this->getUrl()); 

    tntdb::Statement st = conn.prepareCached(
        "select "
        "v.id, v.datum, v.info, v.name"
        "from"
        "v_client_info_last v"
        "where v.id_discovered_device = :id_devicediscovered and v.id_client = :id_client"
        );
    
    //   Should return one row or nothing.
    tntdb::Result result = st.setInt("id_deviceidscovered", deviceDiscoveredId).
                                setInt("id_client", clientId).select();
    int rsize = result.size();
    if ( rsize == 1)
    {
        tntdb::Result::const_iterator it = result.begin();
        tntdb::Row row = *it;

        //fill all fields                                 
        //id
        int tmp;
        row[0].get(tmp);
        this->setId(tmp);
    
        //date
        time_t tmp_t;
        row[1].get(tmp_t);
        this->setTimestampt(tmp_t);

        //blobData
        row[2].get(_blobData);
    
        //clientId
        _clientId = clientId;
        
        //clientName
        row[3].get(_clientName);

        //deviceDiscoveredId
        _deviceDiscoveredId = deviceDiscoveredId;
                        
        this->setState(OS_SELECTED);
    }
    else if (rsize > 1)
    {
        //log this should never hapen
    }
    return rsize;
}

unsigned int
ClientInfo::
db_select_timestampt()
{
    tntdb::Connection conn; 
    conn = tntdb::connectCached(this->getUrl());
    
    tntdb::Statement st = conn.prepareCached(
        " select"
        " v.timestampt"
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
          
        //timestampt
        time_t tmp_t;
        row[0].get(tmp_t);
        this->setTimestampt(tmp_t);

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
// ClientInfo::selectLastRecord(int clientId, int deviceDiscoveredId)
unsigned int ClientInfo::selectLastRecord(std::string clientName, int deviceDiscoveredId)
{
    tntdb::Connection conn; 
    conn = tntdb::connectCached(this->getUrl()); 

    tntdb::Statement st = conn.prepareCached(
        "select "
        "v.id, v.datum, v.info, v.id_client"
        "from"
        "v_client_info_last v"
        "where v.id_deviceDiscovered = :id_devicediscovered and v.client_name = :clientname"
        );
    
    //   Should return one row or nothing.
    tntdb::Result result = st.setInt("id_devicediscovered", deviceDiscoveredId).
                                setString("clientname", clientName).select();
    int rsize = result.size();
    if ( rsize == 1)
    {
        tntdb::Result::const_iterator it = result.begin();
        tntdb::Row row = *it;

        //fill all fields                         
        //id
        int tmp;
        row[0].get(tmp);
        this->setId(tmp);
    
        //date
        time_t tmp_t;
        row[1].get(tmp_t);
        this->setTimestampt(tmp_t);

        //blobData
        row[2].get(_blobData);
    
        //clientName
        _clientName = clientName;
        
        //clientId
        row[3].get(_clientId);

        //deviceDiscoveredId
        _deviceDiscoveredId = deviceDiscoveredId;
                        
        this->setState(OS_SELECTED);
    }
    else if (rsize > 1)
    {
        //log this should never hapen
    }
    return rsize;
}

unsigned int
ClientInfo::
getClientId()
{
    return _clientId;
}

unsigned int
ClientInfo::
getDeviceDiscoveredId()
{
    return _deviceDiscoveredId;
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
    //TODO
    if (this->selectClientName(clientId))
        _clientId = clientId;
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
    //TODO
    _blobData = blobData;
}

void
ClientInfo::
setDeviceDiscoveredId(int deviceDiscoveredId)
{
    //TODO
    _deviceDiscoveredId = deviceDiscoveredId;
}

bool
ClientInfo::
selectClientName(unsigned int clientId)
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

} //end namespace
