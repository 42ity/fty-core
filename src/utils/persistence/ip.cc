#include "ip.h"
#include <tntdb/connect.h>
#include <tntdb/connection.h>

#include <tntdb/row.h>

#include <tntdb/result.h>
namespace utils{

/////////////////////////////////////////////////////////////
////////////////// IP ///////////////////////////////////////
/////////////////////////////////////////////////////////////

Ip* Ip::getLastInfo(std::string url, std::string ip)
{
    tntdb::Connection conn; 
    conn = tntdb::connectCached(url);  // connects to the db

    /**
     * TODO add more columns
     * TODO ip6 char to int
     */
    tntdb::Statement st = conn.prepareCached(
        "select "
        "v.id, v.id_deviceDiscovered, v.datum"
        "from"
        "v_ip_last v"
        "where v.ip = :thisip"
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
        int tmp;
        row[0].get(tmp);
        newIp->setId(tmp);
    
        //deviceDiscoveredId
        row[1].get(tmp);
        newIp->setDeviceDiscoveredId(tmp);
    
        //date
        time_t datetmp;
        row[2].get(datetmp);
        newIp->setDate(datetmp);
            
        /**
          * TODO don't forget read all columns here
          */
        newIp->setState(OS_SELECTED);

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

int Ip::getDeviceDiscoveredId()
{
    return _deviceDiscoveredId;
}

void Ip::setIp(std::string ip)
{
    _ip = CIDRAddress(ip);
}

void Ip::setIp(CIDRAddress ip)
{
    _ip = ip;
}

void Ip::setDeviceDiscoveredId(int deviceDiscoveredId)
{
    _deviceDiscoveredId = deviceDiscoveredId;
}

void Ip::setDate(time_t date)
{
    _date = date;
}

Ip::Ip(std::string url)
:DataBaseObject(url)
{
    _ip = CIDRAddress("");
}


}// namespace utils


