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

/*! \file persistence.cc
    \brief Class for manipulating with database

    While all our modules must behave well and must not do 
    unnecessary actions, for all specific purposes 
    specific functions are prepared.

    
    \author Alena Chernikava <alenachernikava@eaton.com>
*/ 
 
#include <string>
#include <stdlib.h>
//#include "databaseobject.h"
#include "persistence.h"
#include <tntdb/result.h>
#include <tntdb/row.h>
#include <tntdb/value.h>
#include <tntdb/statement.h>
#include "log.h"

namespace utils {

/*
std::map<ObjectState, std::string> ObjectStateMap = {
    {OS_NEW, "new"},
    {OS_SELECTED, "selected"},
    {OS_UPDATED, "updated"},
    {OS_DELETED, "deleted"}
};
*/
////////////////////////////////////////////////////////////
//////////      DataBaseObject               ///////////////
////////////////////////////////////////////////////////////

/*
// is used only for internal porposes. 
void DataBaseObject::setId(int id)
{
    _id = id;
}

void DataBaseObject::setState(ObjectState state)
{
    _state = state;
    if ( _state == OS_NEW)
        _id = -1;
}

DataBaseObject::DataBaseObject(std::string url)
{
    _url = url;
    this->setState(OS_NEW);
}

ObjectState DataBaseObject::getState()
{
    return _state;
}

int DataBaseObject::getId()
{
    if ( _state == OS_NEW )
    {
        if (_id != -1)
        {
            //TODO log(this should never happen)
        }
        _id = -1;
    }
    return _id;
}

std::string DataBaseObject::getUrl()
{
    return _url;
}

// TODO make it protected?
//void DataBaseObject::setUrl(std::string url)
//{
//    _url = url;
//    // TODO drop ID
//}

DataBaseObject::~DataBaseObject()
{
    //TODO
}

std::string DataBaseObject::toString()
{
    std::string tmp = "_url="+_url+";_id="+std::to_string(_id)+";state=";//+utils::dbtoString(_state);
    return tmp; 
}
*/
} // namespace utils

/*
IP
setIp(std::string ip){
            _ip = ip;
            _ip.valid()
        }
        std::string getIp() {
            return _ip.toString();
        }



*/


























/*

// Empty constructor
DataBase::DataBase()
{
    _conn = NULL;
}

// Constructor with all parameters
DataBase::DataBase(std::string db, std::string usr, std::string passwd)
{
    tntdb::Connection* conn = new tntdb::Connection;

    //TODO if MYSQL then
    *conn = tntdb::connect("mysql:db="+db+";user="+usr+";passwd="+passwd);
    //TODO else TODO
    //
    _conn = conn;
}


unsigned int DataBase::insComputerNew(   const std::string name,
                            const int info1, 
                            const int info2,
                            const std::string fingerprint)
{
     tntdb::Statement st = _conn->prepare(
        "insert     into    v_computer( ID , name , info1 , info2 , fingerprint )"
        "           values  (NULL, :vname, :info1 , :vinfo2, :vfingerprint)");

    unsigned int n=st.setString("vname", name) 
        .setInt("vinfo1", info1)
        .setInt("vinfo2", info2)
        .setString("vfingerprint",fingerprint)
        .execute();
    return n;
}

unsigned int DataBase::insComputerUnknown( const int info1, 
                            const int info2,
                            const std::string fingerprint)
{
    return DataBase::insComputerNew("unknown",info1,info2,fingerprint);
}

unsigned int DataBase::selComputerByNameGeneral(std::string name)
{
    tntdb::Statement st ;
    = _conn->prepare(
        "select ID , name , info1 , info2 , fingerprint "
        "       from v_computer where name=:vname");
    
    unsigned int n=st.setString("vname", name)
        .execute();
    return n;
}

//unsigned int selComputersIpByName(NULL/date/-1);
*/

