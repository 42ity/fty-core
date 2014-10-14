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

/*
Author: Alena Chernikava <alenachernikava@eaton.com>
        Karol Hrdina <karolhrdina@eaton.com>
 
Description: ...
References: BIOS-397
*/

/* TODO
 - Ip address is being stored as string at the moment; Store it as two byte arrays (hi, lo).
*/

#include <assert.h>

#include <czmq.h>

#include "cidr.h"
#include "persistence.h"
#include "persistencelogic.h"

#define NETHISTORY_AUTO_CMD     'a'
#define NETHISTORY_MAN_CMD      'm'
#define NETHISTORY_EXCL_CMD     'e'

namespace utils {

namespace db {

char nethistory_id_cmd (int id) {
    assert (id != 0);
    switch (id)
    {
        case NETDISC_MSG_AUTO_ADD:
        {
            return NETHISTORY_AUTO_CMD;
        }
        case NETDISC_MSG_MAN_ADD:
        {
            return NETHISTORY_MAN_CMD;
        }
        case NETDISC_MSG_EXCL_ADD:
        {
            return NETHISTORY_EXCL_CMD;
        }
        default:
        {
            return 0;
        }
    }
}

int nethistory_cmd_id (char cmd) {
    assert (static_cast<int> (cmd) != 0);
    switch (cmd)
    {
        case NETHISTORY_AUTO_CMD:
        {
            return NETHISTORY_AUTO_CMD;
        }
        case NETHISTORY_MAN_CMD:
        {
            return NETHISTORY_MAN_CMD;
        }
        case NETHISTORY_EXCL_CMD:
        {
            return NETHISTORY_EXCL_CMD;
        }
        default:
        {
            return 0;
        }
    }        
}

bool
process_message(const std::string& url, const netdisc_msg_t& msg)
{
    bool result = false;

    // cast away the const - zproto generated methods dont' have const
    netdisc_msg_t& msg_nc = const_cast<netdisc_msg_t&>(msg);
 
    int msg_id          = netdisc_msg_id (&msg_nc);    
    const char *name    = netdisc_msg_name (&msg_nc); 
    const int ipver     = static_cast<int>(netdisc_msg_ipver (&msg_nc));
    const char *ipaddr  = netdisc_msg_ipaddr (&msg_nc);
    int prefixlen       = static_cast<int>(netdisc_msg_prefixlen (&msg_nc));
    std::string mac (netdisc_msg_mac (&msg_nc));
    char command        = nethistory_id_cmd (netdisc_msg_id (&msg_nc));
    CIDRAddress address (ipaddr, prefixlen);

    unsigned int rows_affected = 0;

    utils::db::NetHistory nethistory(url);
    nethistory.setAddress(address);
    nethistory.setCommand(command);      
    nethistory.setName(name);
    nethistory.setMac(mac);
     
    int id_unique = nethistory.checkUnique();

    switch (msg_id) {

        case NETDISC_MSG_AUTO_ADD:
        {  
            if (id_unique == -1) 
            { 
                rows_affected = nethistory.dbsave();
            
                if (rows_affected == 1)     // if checks didn't pass, then nothing would be inserted
                    result = true;
            }
            else
                result = true;
            break;
        }
        case NETDISC_MSG_AUTO_DEL:
        {           
            if (id_unique != -1)  
            {           
                rows_affected = nethistory.deleteById (id_unique);
                assert (rows_affected == 1);
            }
            result = true;
            break;
         
        }
        case NETDISC_MSG_MAN_ADD:
        {
            if (id_unique == -1) { 
                rows_affected = nethistory.dbsave();
                if (rows_affected == 1)
                    result = true;
            }
            else
                result = true;
            break;
        }
        case NETDISC_MSG_MAN_DEL:
        {
            if (id_unique != -1) { 
                rows_affected = nethistory.deleteById(id_unique);
                assert (rows_affected == 1);
            }
            result = true;
            break;
        }
        case NETDISC_MSG_EXCL_ADD:
        {
            if (id_unique == -1) { 
                rows_affected = nethistory.dbsave();
                if (rows_affected == 1)
                    result = true;
            }
            else
                result = true;
            break;

        }
        case NETDISC_MSG_EXCL_DEL:
        {
            if (id_unique != -1) { 
                rows_affected = nethistory.deleteById(id_unique);
                assert (rows_affected == 1);
            }
            result = true;
            break;

        }
        default:
        {
            result = false;
            break;
        }
    }       
    return result;
};

} // namespace utils

} // namespace db

