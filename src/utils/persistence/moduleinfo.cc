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

ClientInfo::
ClientInfo(std::string url)
:DataBaseObject(url)
{
    _clientName = "";
    _clientId = -1;
    _deviceDiscoveredId = -1;
    _blobData = "";
    _date = time(NULL);
}

ClientInfo::
ClientInfo(std::string url, std::string clientName)
:DataBaseObject(url)
{
    _clientName = clientName;
    _clientId = Client::selectId(url, clientName);
    _deviceDiscoveredId = -1;
    _blobData = "";
    _date = time(NULL);
}

std::string 
ClientInfo::
toString()
{
    return DataBaseObject::toString() + ";" +
                            "clientId=" + std::to_string(_clientId)           + ";" +
                            "clientName"+ _clientName         + ";" +
                            "deviceId=" + std::to_string(_deviceDiscoveredId) + ";" +
                            //"date="     + _date               + ";" +
                            "blobdata=" + _blobData;
}

unsigned int ModuleInfo::selectLastRecord()
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
                         setString("ext",_blob).
                         execute();
    
    if ( n == 1 )
    {
        int newId =  conn.lastInsertId();
        this->setId(newId);
    }
    
    //We don't want get back to application level time of inserting.
     
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
        " v_bios_device_discovered "
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

// if any changes would be made here check if the same need to do int
// ModuleInfo::selectLastRecord(std::string clientName, int deviceDiscoveredId)
unsigned int ModuleInfo::selectLastRecord(int clientId, int deviceDiscoveredId)
{
    tntdb::Connection conn; 
    conn = tntdb::connectCached(this->getUrl()); 

    tntdb::Statement st = conn.prepareCached(
        "select "
        "v.id, v.datum, v.info, v.name"
        "from"
        "v_client_info_last v"
        "where v.id_deviceDiscovered = :id_deviceDiscovered and v.id_client = :id_client"
        );
    
    //   Should return one row or nothing.
    tntdb::Result result = st.setInt("id_deviceDiscovered", deviceDiscoveredId).
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
        row[1].get(_date);

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

// if any changes would be made here check if the same need to do int
// ModuleInfo::selectLastRecord(int clientId, int deviceDiscoveredId)
unsigned int ModuleInfo::selectLastRecord(std::string clientName, int deviceDiscoveredId)
{
    tntdb::Connection conn; 
    conn = tntdb::connectCached(this->getUrl()); 

    tntdb::Statement st = conn.prepareCached(
        "select "
        "v.id, v.datum, v.info, v.id_client"
        "from"
        "v_client_info_last v"
        "where v.id_deviceDiscovered = :id_deviceDiscovered and v.client_name = :clientname"
        );
    
    //   Should return one row or nothing.
    tntdb::Result result = st.setInt("id_deviceDiscovered", deviceDiscoveredId).
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
        row[1].get(_date);

        //blobData
        row[2].get(_blobData);
    
        //clientName
        _clientName = clientName;
        
        //clientId
        row[3].get(_clientId);

        //deviceDiscoveredId
        _deviceDiscoveredId = deviceDiscoveredId;
                        
        this->setState(OS_SELECTED);
        return 1;
    }
    else if (rsize > 1)
    {
        //log this should never hapen
    }
    return rsize;
}

}
