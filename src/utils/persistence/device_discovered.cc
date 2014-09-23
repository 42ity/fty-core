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

DeviceDiscovered::
DeviceDiscovered(std::string url)
    :DataBaseObject(url)
{
    _clientId = Client::selectId(url,_clientName);
    _name = "unknown";
}

DeviceDiscovered::
DeviceDiscovered(std::string url, std::string name)
    :DataBaseObject(url)
{
    _clientId = Client::selectId(url,_clientName);
    _name = name;
}

std::string 
DeviceDiscovered::
toString()
{
    return DataBaseObject::toString() + ";" + "name=" + _name;
}

void 
DeviceDiscovered::
setName(std::string name)
{
    if ( (_name != name) && (this->getState() != OS_DELETED) )
    {
        switch (this->getState()){
            case OS_SELECTED: 
                this->setState(OS_UPDATED);
            case OS_UPDATED:
            case OS_NEW:
            _name = name;
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
        return true;
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
        " v_bios_device_discovered (id,name)"
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
DeviceDiscovered::
db_delete()
{
    tntdb::Connection conn;  
    conn = tntdb::connectCached(this->getUrl());

    tntdb::Statement st = conn.prepareCached(
        " delete from"
        " v_bios_device_discovered "
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
        " v_bios_device_discovered"
        " set name = :name"     //, aaa = :aa
        " where id = :id"
        );
    
    // update one row or nothing
    unsigned int n  = st.setString("name", this->getName()).setInt("id",this->getId()).execute();
    
    // n is 0 or 1
    return n;
}

std::vector<DeviceDiscovered> 
DeviceDiscovered::
selectByName(std::string url, std::string name)
{
    tntdb::Connection conn;  
    conn = tntdb::connectCached(url);  // connects to the db

    /**
     * TODO add more columns
     */
    tntdb::Statement st = conn.prepareCached(
        " select"
        " id"
        " from"
        " v_bios_device_discovered v"
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
            int tid;
            row[0].get(tid);  // read column #0 into variable tid
            newdeviceDiscovered->setId(tid);
            
            //name
            newdeviceDiscovered->setName(name);
            
            /**
             * TODO don't forget read all columns here
             */
            
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
selectById(unsigned int id)
{
    tntdb::Connection conn; 
    conn = tntdb::connectCached(this->getUrl());  // connects to the db
    /**
     * TODO add more columns
     */
    tntdb::Statement st = conn.prepareCached(
        " select"
        " name"
        " from"
        " v_bios_device_discovered v"
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
        row[0].get(_name);  // read column #0 into variable _name
    
        /**
         * TODO don't forget read all columns here
         */
                
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
    if ( this->getState() != ObjectState::OS_NEW )
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

} //end of namespace
