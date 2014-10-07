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

/*! \file NetHistory.cc
    \brief Class for manipulating with database table t_bios_net_history

    \author Alena Chernikava <alenachernikava@eaton.com>
*/ 
 
#include <string>
#include <stdlib.h>
#include <tntdb/result.h>
#include <tntdb/row.h>
#include <tntdb/value.h>
#include <tntdb/statement.h>
#include <tntdb/error.h>
#include "log.h"
#include "nethistory.h"
 
namespace utils {
   
////////////////////////////////////////////////////////////
//////////          NetHistory                 ///////
////////////////////////////////////////////////////////////

void
NetHistory::
clear_this()
{
    _mac =  "";
    _mask = 0;
    _ip = CIDRAddress();
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
NetHistory(std::string url)
    :DataBaseTimeObject(url)
{
    this->clear_this();
}

std::string 
NetHistory::
toString()
{
    return DataBaseTimeObject::toString()         + ";" +
             "mac="       + _mac                  + ";" +
             "mask="      + std::to_string(_mask) + ";" +
             "ip="        + _ip.toString()        + ";" +
             "command="   + _command              + ";" +
             "name="      + _name                 ; 
}

NetHistory::
~NetHistory()
{
    //TODO
}

bool
NetHistory::
check()
{
    if ((_name.length() <= this->getNamesLength()) &&
        this->check_command() )
        return true;
    else
        return false;
}

bool
NetHistory::
check_command()
{
    if  ( ( _command == 'a') || ( _command == 'd') )
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
        " values (NULL,:command,:mask, conv(:mac, 16, 10), NOW(),:ip, :name)"
        );
    
    // Insert one row or nothing
    unsigned int n  = st.setChar("command", _command).
                         setInt("mask",_mask).
                         setString("mac",_mac).
                         setString("ip",_ip.toString()).
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
    
    tntdb::Connection conn;  
    conn = tntdb::connectCached(this->getUrl());

    tntdb::Statement st = conn.prepareCached(
        " update"
        " v_bios_net_history"
        " set ip = :ip, mac = conv(:mac,16,10) , mask = :mask , command = :command , name = :name"     //, aaa = :aa
        " where id = :id"
        );
    
    // update one row or nothing
    unsigned int n  = st.setString("ip", _ip.toString()).
                         setInt("mask",_mask).
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
    conn = tntdb::connectCached(this->getUrl());  // connects to the db
    /**
     * TODO add more columns
     */
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
        _ip = CIDRAddress(tmp_ip);

        //mask
        row[1].get(_mask);

        //mac
        row[2].get(_mac);

        //command
        row[3].get(_command);

        //timestamp
        time_t tmp_t = time(NULL);  // TODO if get-method got NULL, than it doesn't modify variable. So need to define initial value.
                                       // but it should never happen, while this column must be NOT NULL
        bool isNotNull = row[4].get(tmp_t);
        if (isNotNull)
            this->setTimestamp(tmp_t);
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

int 
NetHistory::
getMask()
{
    return _mask;
}

CIDRAddress 
NetHistory::
getIp()
{
    return _ip;
}

char 
NetHistory::
getCommand()
{
    return _command;
}

std::string 
NetHistory::
getMac()
{
    return _mac;
}

std::string 
NetHistory::
getName()
{
    return _name;
}

void
NetHistory::
setName(std::string name)
{
    if ( (_name != name) && (this->getState() != ObjectState::OS_DELETED) )
    {
        switch (this->getState()){
            case ObjectState::OS_SELECTED:
            case ObjectState::OS_INSERTED:
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
setMask(int mask)
{
    if ( (_mask != mask) && (this->getState() != ObjectState::OS_DELETED) )
    {
        switch (this->getState()){
            case ObjectState::OS_SELECTED:
            case ObjectState::OS_INSERTED:
                this->setState(ObjectState::OS_UPDATED);
            case ObjectState::OS_UPDATED:
            case ObjectState::OS_NEW:
                 _mask = mask;
                 break;
            default:
                // TODO log this should never happen
                break;
        }
    }
}

void
NetHistory::
setMac(std::string mac)
{
    if ( (_mac != mac) && (this->getState() != ObjectState::OS_DELETED) )
    {
        switch (this->getState()){
            case ObjectState::OS_SELECTED:
            case ObjectState::OS_INSERTED:
                this->setState(ObjectState::OS_UPDATED);
            case ObjectState::OS_UPDATED:
            case ObjectState::OS_NEW:
                 _mac = mac;
                 break;
            default:
                // TODO log this should never happen
                break;
        }
    }
}


void
NetHistory::
setIp(CIDRAddress ip)
{
    if (  (!(_ip == ip)) && (this->getState() != ObjectState::OS_DELETED) )
    {
        switch (this->getState()){
            case ObjectState::OS_SELECTED:
            case ObjectState::OS_INSERTED:
                this->setState(ObjectState::OS_UPDATED);
            case ObjectState::OS_UPDATED:
            case ObjectState::OS_NEW:
                 _ip = ip;
                 break;
            default:
                // TODO log this should never happen
                break;
        }
    }  
}

void 
NetHistory::
setIp(std::string ip)
{
    CIDRAddress tmp_ip(ip);
    if (  (_ip != tmp_ip ) && (this->getState() != ObjectState::OS_DELETED) )
    {
        switch (this->getState()){
            case ObjectState::OS_SELECTED:
            case ObjectState::OS_INSERTED:
                this->setState(ObjectState::OS_UPDATED);
            case ObjectState::OS_UPDATED:
            case ObjectState::OS_NEW:
                 _ip = tmp_ip;
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
    if ( (_command != command) && (this->getState() != ObjectState::OS_DELETED) )
    {
        switch (this->getState()){
            case ObjectState::OS_SELECTED:
            case ObjectState::OS_INSERTED:
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
        tntdb::Row row = st.setInt("id", this->getId()).selectRow();
          
        
        //timestamp
        time_t tmp_t = time(NULL);  // TODO if get-method got NULL, than it doesn't modify variable. So need to define initial value.
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


} //end of namespace utils
