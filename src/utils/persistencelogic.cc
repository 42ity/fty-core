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
#include "log.h"

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
process_message(const std::string& url, zmsg_t *msg) {
    log_open();
    log_set_level(LOG_DEBUG);
    log_set_syslog_level(LOG_DEBUG);
    log_info ("%s", "process_message() start\n");

    zmsg_t *msg2;

    //XXX: this is ugly, however we need to distinguish between types of messages
    //     zccp will solve that by DIRECT message
    msg2 = zmsg_dup(msg);
    netdisc_msg_t *netdisc_msg = netdisc_msg_decode(&msg2);
    if (netdisc_msg) {
        //TODO: check the log level!
        netdisc_msg_print(netdisc_msg);
        return netdisc_msg_process(url, *netdisc_msg);
    }

    log_error("unsupported message type, skipped!\n");
    log_info ("%s", "process_message() end\n");
    log_close ();
    return false;
}

bool
netdisc_msg_process(const std::string& url, const netdisc_msg_t& msg)
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
                assert (rows_affected == 1);            
            }
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
                assert (rows_affected == 1);
            }
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
                assert (rows_affected == 1);
            }
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
        // Example: Let's suppose we are listening on a ROUTER socket from a range of producers.
        //          Someone sends us a message from older protocol that has been dropped.
        //          Are we going to return 'false', that usually means db fatal error, and
        //          make the caller crash/quit? OR does it make more sense to say, OK, message
        //          has been processed and we'll log a warning about unexpected message type
            result = true;        
            log_warning ("Unexpected message type received; message id = '%d'", static_cast<int>(msg_id));        
            break;
        }
        
    }
    return result;
};

} // namespace utils

} // namespace db

