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

/*! \file nethistory.cc
    \brief Class for manipulating with database table t_bios_net_history

    \author Alena Chernikava <alenachernikava@eaton.com>
*/ 
 
#include <string>
#include <stdlib.h>
#include <algorithm>

#include <tntdb/result.h>
#include <tntdb/row.h>
#include <tntdb/value.h>
#include <tntdb/statement.h>
#include <tntdb/error.h>
#include <cxxtools/regex.h>

#include "log.h"
#include "nethistory.h"

 
namespace utils {

namespace db {


//-----------------------------------------------------------

//Internal function for remove colons from mac address
void
_removeColonMac(std::string &newmac)
{
    newmac.erase (std::remove (newmac.begin(), newmac.end(), ':'), newmac.end()); 
}

//Internal function for add colons to mac address
const std::string
_addColonMac(const std::string &mac)
{
    std::string macWithColons(mac);
    macWithColons.insert(2,1,':');
    macWithColons.insert(5,1,':');
    macWithColons.insert(8,1,':');
    macWithColons.insert(11,1,':');
    macWithColons.insert(14,1,':');
    return macWithColons;
}

//Internal method:check whether the mac address has right format
bool
checkMac (const std::string &mac_address)
{
    cxxtools::Regex regex("^[0-9,a-f,A-F][0-9,a-f,A-F]:[0-9,a-f,A-F][0-9,a-f,A-F]:[0-9,a-f,A-F][0-9,a-f,A-F]:[0-9,a-f,A-F][0-9,a-f,A-F]:[0-9,a-f,A-F][0-9,a-f,A-F]:[0-9,a-f,A-F][0-9,a-f,A-F]$");
    if(!regex.match(mac_address))
        return false;
    else
        return true;
}
//-----------------------------------------------------------

void
NetHistory::
clear_this()
{
    _mac =  "";
    _address = CIDRAddress();
    _command = 'z';
    _name = "";
}

void
NetHistory::
clear(){
    DataBaseTimeObject::clear();
    this->clear_this();
}

NetHistory::
NetHistory(const std::string &url)
    :DataBaseTimeObject(url)
{
    this->clear_this();
}

std::string 
NetHistory::
toString() const
{
    return DataBaseTimeObject::toString()         + ";" +
             "mac="       + this->getMac()        + ";" +
             "address="   + _address.toString()   + ";" +
             "command="   + _command              + ";" +
             "name="      + _name                 ; 
}
  
NetHistory::
~NetHistory()
{
}

const std::string 
NetHistory::
getMac() const
{
    if (_mac != "")    
        return utils::db::_addColonMac(_mac);
    else
        return "";
}

bool
NetHistory::
check() const
{
    if ((_name.length() <= this->getNamesLength()) &&
        this->check_command() && _address.valid() )
        return true;        // in case of prefixlen = 0, the address would be invalid
    else
        return false;
}

bool
NetHistory::
check_command() const
{

    // TODO change hardcoded constants to DEFINE
    // in future refinement
    if  ( ( _command == 'a') || ( _command == 'm') || ( _command == 'e') )
        return true;
    else
        return false;

}

unsigned int 
NetHistory::
db_insert()
{
    
    tntdb::Connection conn;  
    conn = tntdb::connectCached(this->getUrl());

    tntdb::Statement st = conn.prepareCached(
        " insert into"
        " v_bios_net_history (id,command,mask,mac,timestamp,ip,name)"
        " values (NULL,:command,:mask, conv(:mac, 16, 10), UTC_TIMESTAMP(),:ip, :name)"
        );
    
    // Insert one row or nothing
    unsigned int n  = st.setChar("command", _command).
                         setInt("mask",_address.prefix()).
                         setString("mac",_mac).
                         setString("ip",_address.toString(CIDROptions::CIDR_WITHOUT_PREFIX)).
                         setString("name",_name).
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
NetHistory::
db_delete()
{
    tntdb::Connection conn;  
    conn = tntdb::connectCached(this->getUrl());

    tntdb::Statement st = conn.prepareCached(
        " delete from"
        " v_bios_net_history "
        " where id = :id"
        );
    
    // Delete one row or nothing
    unsigned int n  = st.setInt("id", this->getId()).execute();
    
    // n is 0 or 1
    return n;
}

unsigned int 
NetHistory::
db_update()
{
    // Now we are not going to update existing records. 
    // Don't update the timpestamp now.
    tntdb::Connection conn;  
    conn = tntdb::connectCached(this->getUrl());

    tntdb::Statement st = conn.prepareCached(
        " update"
        " v_bios_net_history"
        " set ip = :ip, mac = conv(:mac,16,10) , mask = :mask , command = :command , name = :name"     //, aaa = :aa
        " where id = :id"
        );
    
    // update one row or nothing
    unsigned int n  = st.setString("ip", _address.toString(CIDROptions::CIDR_WITHOUT_PREFIX)).
                         setInt("mask",_address.prefix()).
                         setString("mac", _mac).
                         setChar("command",_command).
                         setString("name",_name).
                         execute();
    
    // n is 0 or 1
    return n;
}

unsigned int 
NetHistory::
selectById(int id)
{
    tntdb::Connection conn; 
    conn = tntdb::connectCached(this->getUrl());
    
    tntdb::Statement st = conn.prepareCached(
        " select"
        " ip,mask,conv(mac,10,16),command,timestamp,name"
        " from"
        " v_bios_net_history v"
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
        std::string tmp_ip = "";
        row[0].get(tmp_ip);

        //mask
        int tmp_i = -1;
        row[1].get(tmp_i);

        //address
        _address = CIDRAddress(tmp_ip,tmp_i);
        _address = _address.network(); // put in network format, to be sure it is in network format

        //mac
        row[2].getString(_mac);

        //command
        row[3].get(_command);

        //timestamp
        tntdb::Datetime mydatetime;
        bool isNotNull = row[4].get(mydatetime);
        if (isNotNull)
            this->setTimestamp(utils::db::convertToCTime(mydatetime));
        else
        {
            //TODO
            //log THIS SHOULD NEVER HAPPEN
        }
        
        //name
        row[5].get(_name);
        
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
NetHistory::
setName(const std::string& name)
{
    if ( ( _name != name ) && ( this->getState() != ObjectState::OS_DELETED )
         && ( this->getState() != ObjectState::OS_INSERTED ) )
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
}

void
NetHistory::
setMac(const std::string& mac_address)
{
    if (checkMac(mac_address))
    {
        std::string macc(mac_address);
        utils::db::_removeColonMac(macc);
    
        if ( ( _mac != macc ) && ( this->getState() != ObjectState::OS_DELETED ) 
            && ( this->getState() != ObjectState::OS_INSERTED ) )
        {
            switch (this->getState()){
                case ObjectState::OS_SELECTED:
                    this->setState(ObjectState::OS_UPDATED);
                case ObjectState::OS_UPDATED:
                case ObjectState::OS_NEW:
                     _mac = macc;
                     break;
                default:
                    // TODO log this should never happen
                    break;
            }
        }
    }
}


void
NetHistory::
setAddress(const CIDRAddress& cidr_address)
{
    // We are not sure, if the passed address is in a network format, so convert it now
    CIDRAddress newaddr = cidr_address.network();  
    if ( ( _address != newaddr ) && ( this->getState() != ObjectState::OS_DELETED ) 
        && ( this->getState() != ObjectState::OS_INSERTED ) )
    {
        switch (this->getState()){
            case ObjectState::OS_SELECTED:
                this->setState(ObjectState::OS_UPDATED);
            case ObjectState::OS_UPDATED:
            case ObjectState::OS_NEW:
                 _address = newaddr;
                 break;
            default:
                // TODO log this should never happen
                break;
        }
    }  
}

void 
NetHistory::
setCommand(char command)
{
    if ( (_command != command) && (this->getState() != ObjectState::OS_DELETED) 
        && ( this->getState() != ObjectState::OS_INSERTED ) )
    {
        switch (this->getState()){
            case ObjectState::OS_SELECTED:
                this->setState(ObjectState::OS_UPDATED);
            case ObjectState::OS_UPDATED:
            case ObjectState::OS_NEW:
                 _command = command;
                 break;
            default:
                // TODO log this should never happen
                break;
        }
    }  
}

unsigned int
NetHistory::
db_select_timestamp()
{
    tntdb::Connection conn; 
    conn = tntdb::connectCached(this->getUrl());
    
    tntdb::Statement st = conn.prepareCached(
        " select"
        " v.timestamp"
        " from"
        " v_bios_net_history v"
        " where v.id = :id"
        );
    
    /**
     *  Can return one row or nothing
     */
    int n;
    try{
        tntdb::Value value = st.setInt("id", this->getId()).selectValue();
          
        //timestamp
        tntdb::Datetime mydatetime;
        bool isNotNull = value.get(mydatetime);
        if (isNotNull)
            this->setTimestamp(utils::db::convertToCTime(mydatetime));
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
int
NetHistory::
checkUnique() const
{
    if ( _command == 'a' )
        return this->checkUniqueAuto();
    else if ( _command == 'm' )
        return this->checkUniqueManual();
    else if ( _command == 'e' )
        return this->checkUniqueExclude();
    return -1;
}


int 
NetHistory::
checkUniqueExclude() const
{
    return this->checkUniqueManual();
}

int 
NetHistory::
checkUniqueManual() const
{
    tntdb::Connection conn; 
    conn = tntdb::connectCached(this->getUrl());
    
    tntdb::Statement st = conn.prepareCached(
        " select"
        " id"
        " from"
        " v_bios_net_history v"
        " where v.command = :command and v.ip = :ip and v.mask = :mask"
        );
    //It must be called only for commands 'e' and  'm'
    tntdb::Result result = st.setChar("command", _command).
                              setString("ip", _address.
                              toString(CIDROptions::CIDR_WITHOUT_PREFIX)).
                              setInt("mask",_address.prefix()).
                              select();
    if (result.empty()) {
        return -1;
    }        
    tntdb::Row row = result.getRow(0);
    int ret_val = -1;
    row[0].get(ret_val);
    return ret_val;
}


int 
NetHistory::
checkUniqueAuto() const
{
    tntdb::Connection conn; 
    conn = tntdb::connectCached(this->getUrl());
    
    tntdb::Statement st = conn.prepareCached(
        " select"
        " id"
        " from"
        " v_bios_net_history v"
        " where v.command = :command and v.ip = :ip and v.mask = :mask"
        "       and  v.mac = conv(:mac, 16, 10) and v.name = :name"
        );

    //It must be called only for commands 'a'
    tntdb::Result result = st.setChar("command", _command).
                              setString("ip", _address.
                              toString(CIDROptions::CIDR_WITHOUT_PREFIX)).
                              setInt("mask",_address.prefix()).
                              setString("mac",_mac).
                              setString("name",_name).
                              select();
    if (result.empty()) {
        return -1;
    }        
    tntdb::Row row = result.getRow(0);
    int ret_val = -1;
    row[0].get(ret_val);
    return ret_val;
}


std::vector<NetHistory>
NetHistory::
getHistory(const std::string& url)
{
    
    tntdb::Connection conn;  
    conn = tntdb::connectCached(url); 

    tntdb::Statement st = conn.prepareCached(
        " select"
        " v.id, v.name , conv(v.mac,10,16), v.ip , v.mask, v.command , v.timestamp"
        " from"
        " v_bios_net_history v"
        );

    tntdb::Result result = st.select();

    std::vector<NetHistory> history;

    int rsize = result.size();
    if ( rsize > 0 )
    {
        //There are some records
        for ( tntdb::Result::const_iterator it = result.begin();
                it != result.end(); ++it)
        {
            tntdb::Row row = *it;

            NetHistory* newNetHistory = new NetHistory(url);
            
            //id
            int tmp_i = -1;
            row[0].get(tmp_i);
            newNetHistory->setId(tmp_i);

            //name
            std::string tmp_s = "";
            row[1].get(tmp_s);
            newNetHistory->setName(tmp_s);

            //mac
            tmp_s = "";
            row[2].getString(tmp_s);
            //TODO check if mac is ok
            newNetHistory->setMac(_addColonMac(tmp_s));
                    
            //ip
            tmp_s = "";
            row[3].get(tmp_s);

            //mask
            tmp_i = -1;
            row[4].get(tmp_i);

            //address
            CIDRAddress address = CIDRAddress(tmp_s,tmp_i);
            // put in network format, to be sure it is in network format
            address = address.network();
            newNetHistory->setAddress(address);

            //command
            char tmp_c = 'z';
            row[5].get(tmp_c);
            newNetHistory->setCommand(tmp_c);


            //timestamp
            tntdb::Datetime mydatetime;
            bool isNotNull = row[6].get(mydatetime);
            if (isNotNull)
                newNetHistory->setTimestamp(utils::db::convertToCTime(mydatetime));
            else
            {
                //TODO
                //log THIS SHOULD NEVER HAPPEN
            }
            
            //state
            newNetHistory->setState(ObjectState::OS_SELECTED);
            
            //TODO insert only if valid
            //TODO redisign mac check
            history.push_back(*newNetHistory);
        }
    }
    //If no rows were selected, return empty vector
    return history;
}

} // namespace db

} //end of namespace utils
