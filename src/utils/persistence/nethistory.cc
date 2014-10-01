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
    return DataBaseTimeObject::toString() + ";" +
             "mac=" + _mac + ";" +
             "mask=" + std::to_string(_mask) +";" +
             "ip" + _ip.toString() +";" +
             "command" +_command;
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
    return true;
}


unsigned int 
NetHistory::
db_insert()
{
    
    tntdb::Connection conn;  
    conn = tntdb::connectCached(this->getUrl());

    tntdb::Statement st = conn.prepareCached(
        " insert into"
        " v_bios_net_history (id,command,mask,mac,timestamp,ip)"
        " values (NULL,:command,:mask,:mac, NOW(),:ip)"
        );
    
    // Insert one row or nothing
    unsigned int n  = st.setChar("command", _command).
                         setInt("mask",_mask).
                         setString("mac",_mac).
                         setString("ip",_ip.toString()).
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
        " set ip = :ip, mac = :mac , mask = :mask , command = :command"     //, aaa = :aa
        " where id = :id"
        );
    
    // update one row or nothing
    unsigned int n  = st.setString("ip", _ip.toString()).
                         setInt("mask",_mask).
                         setString("mac", _mac).
                         setChar("command",_command).
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
        " ip,mask,mac,command,timestamp"
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

        //timestamp
        time_t tmp_t = time(nullptr);  // TODO if get-method got NULL, than it doesn't modify variable. So need to define initial value.
                                       // but it should never happen, while this column must be NOT NULL
        bool isNotNull = row[3].get(tmp_t);
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


void
NetHistory::
setMask(int mask)
{
    //TODO
    _mask = mask;
}

void
NetHistory::
setIp(CIDRAddress ip)
{//TODO
    _ip = ip;
}

void 
NetHistory::
setCommand(char command)
{
    //TODO
    _command = command;
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
        time_t tmp_t = time(nullptr);  // TODO if get-method got NULL, than it doesn't modify variable. So need to define initial value.
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
