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

/*! \file ip.h
    
    \brief Realisation of the class for manipulating with the database 
    objects stored in the table t_bios_discoverd_ip
 
    \author Alena Chernikava <alenachernikava@eaton.com>
*/ 
#include "ip.h"
#include <tntdb/connect.h>
#include <tntdb/connection.h>

#include <tntdb/row.h>

#include <tntdb/result.h>
#include <tntdb/error.h>

namespace utils{

/////////////////////////////////////////////////////////////
//////////           IP          ////////////////////////////
/////////////////////////////////////////////////////////////

bool
Ip::
check()
{
    return true;
}


unsigned int 
Ip::
db_insert()
{
    
    tntdb::Connection conn;  
    conn = tntdb::connectCached(this->getUrl());

    tntdb::Statement st = conn.prepareCached(
        " insert into"
        " v_bios_discovered_ip (id,ip,timestamp,id_discovered_device)"
        " values (NULL, :ip, NOW(),:discovereddeviceid)"
        );
    
    // Insert one row or nothing
    unsigned int n  = st.setString("ip", _ip.toString()).
                         setInt("discovereddeviceid",_deviceDiscoveredId).execute();
    
    if ( n == 1 )
    {
        int newId =  conn.lastInsertId();
        this->setId(newId);
    }
    
    // n is 0 or 1
    return n;
}

unsigned int 
Ip::
db_delete()
{
    tntdb::Connection conn;  
    conn = tntdb::connectCached(this->getUrl());

    tntdb::Statement st = conn.prepareCached(
        " delete from"
        " v_bios_discovered_ip "
        " where v.id = :id"
        );
    
    // Delete one row or nothing
    unsigned int n  = st.setInt("id", this->getId()).execute();
    
    // n is 0 or 1
    return n;
}

unsigned int 
Ip::
db_update()
{
    
    tntdb::Connection conn;  
    conn = tntdb::connectCached(this->getUrl());

    tntdb::Statement st = conn.prepareCached(
        " update"
        " v_bios_discovered_ip"
        " set v.ip = :ip , v.id_discovered_device = :discovereddeviceid" 
        " where id = :id"
        );
    
    // update one row or nothing
    unsigned int n  = st.setInt("id", this->getId()).
                         setString("ip", _ip.toString()).
                         setInt("discovereddeviceid",_deviceDiscoveredId).
                         execute();
    
    // n is 0 or 1
    return n;
}

unsigned int 
Ip::
selectById(unsigned int id)
{
    tntdb::Connection conn; 
    conn = tntdb::connectCached(this->getUrl());  // connects to the db
    /**
     * TODO add more columns
     */
    tntdb::Statement st = conn.prepareCached(
        " select"
        " ip, id_discovered_device, timestamp"
        " from"
        " v_bios_discovered_ip v"
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
        
        //ip
        std::string tmp_str="";
        row[0].get(tmp_str);
        _ip = CIDRAddress(tmp_str);
    
        //id_discovereddevice
        row[1].get(_deviceDiscoveredId);

        //timestamp
        time_t tmp_t ;  // TODO if get-method got NULL, than it doesn't modify variable. So need to define initial value
        row[1].get(tmp_t);
        this->setTimestamp(tmp_t);
                
        //state
        this->setState(ObjectState::OS_SELECTED);
        
        n = 1; 
    }
    catch (const tntdb::NotFound &e){
        n = 0;
    }
    return n;
}



Ip* 
Ip::getLastInfo(std::string url, std::string ip)
{
    tntdb::Connection conn; 
    conn = tntdb::connectCached(url);  // connects to the db

    /**
     * TODO add more columns
     * TODO ip6 char to int
     */
    tntdb::Statement st = conn.prepareCached(
        " select "
        " v.id, v.id_deviceDiscovered, v.datum"
        " from"
        " v_ip_last v"
        " where v.ip = :thisip"
        );
    
    /**
     *  Can return one row or nothing
     */
    tntdb::Result result = st.setString("thisip", ip).select();
    if ( result.size() == 1)
    {
        tntdb::Result::const_iterator it = result.begin();
    
        tntdb::Row row = *it;
        
        Ip* newIp = new Ip(url);
            
        //ip
        newIp->setIp(ip);
           
        //id
        int tmp = -1;
        row[0].get(tmp);
        newIp->setId(tmp);
    
        //deviceDiscoveredId
        tmp = -1;
        row[1].get(tmp);
        newIp->setDeviceDiscoveredId(tmp);
    
        //date
        time_t datetmp;  // TODO if get-method got NULL, than it doesn't modify variable. So need to define initial value
        row[2].get(datetmp);
        newIp->setTimestamp(datetmp);
            
        /**
          * TODO don't forget read all columns here
          */
        newIp->setState(ObjectState::OS_SELECTED);

        return newIp;

    }
    else 
    {
        if (result.size() > 1) 
        {
                //TODO log(this shoul never happen)
        }
        return NULL;
    }
}

int 
Ip::
getDeviceDiscoveredId()
{
    return _deviceDiscoveredId;
}

void 
Ip::
setIp(std::string ip)
{
    _ip = CIDRAddress(ip);
}

void 
Ip::
setIp(CIDRAddress ip)
{
    _ip = ip;
}

void 
Ip::
setDeviceDiscoveredId(int deviceDiscoveredId)
{
    _deviceDiscoveredId = deviceDiscoveredId;
}

Ip::
Ip(std::string url)
    :DataBaseTimeObject(url)
{
    _ip = CIDRAddress("");
}


}// namespace utils


