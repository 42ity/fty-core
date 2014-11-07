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
#include "monitor.h"
#include "log.h"

#define NETHISTORY_AUTO_CMD     'a'
#define NETHISTORY_MAN_CMD      'm'
#define NETHISTORY_EXCL_CMD     'e'

namespace persist {

char nethistory_id_cmd (int id) {
    assert (id != 0);
    switch (id)
    {
        case NETDISC_MSG_AUTO_ADD:
        case NETDISC_MSG_AUTO_DEL:
        {
            return NETHISTORY_AUTO_CMD;
        }
        case NETDISC_MSG_MAN_ADD:
        case NETDISC_MSG_MAN_DEL:
        {
            return NETHISTORY_MAN_CMD;
        }
        case NETDISC_MSG_EXCL_ADD:
        case NETDISC_MSG_EXCL_DEL:
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
    zmsg_print(msg);
    msg2 = zmsg_dup(msg);
    zmsg_pop(msg2);
    assert(msg2);
    netdisc_msg_t *netdisc_msg = netdisc_msg_decode(&msg2);
    if (netdisc_msg) {
        //TODO: check the log level!
        netdisc_msg_print(netdisc_msg);
        return netdisc_msg_process(url, *netdisc_msg);
    }
    msg2 = zmsg_dup(msg);
    zmsg_pop(msg2);
    assert(msg2);
    powerdev_msg_t *powerdev_msg = powerdev_msg_decode(&msg2);
    if (powerdev_msg) {
        //TODO: check the log level!
        powerdev_msg_print(powerdev_msg);
        return powerdev_msg_process(url, *powerdev_msg);
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
    assert(command);    // fail on unsupported type
    utils::CIDRAddress address (ipaddr, prefixlen);

    unsigned int rows_affected = 0;

    persist::NetHistory nethistory(url);
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

/**
 * \brief processes the powerdev_msg messages
 *
 * \param url - a connection to the database
 * \param msg - a message to be processed
 *
 * \return  true  - if message was processed successfully
 *          false - if message was ignored 
 */
bool
powerdev_msg_process (const std::string& url, const powerdev_msg_t& msg)
{
    powerdev_msg_t& msg_c = const_cast<powerdev_msg_t&>(msg);
    int msg_id            = powerdev_msg_id (&msg_c);

    bool result = false;
    switch (msg_id) {
        
        case POWERDEV_MSG_POWERDEV_STATUS:
        {
            const char* devicename = powerdev_msg_deviceid (&msg_c);
            const char* devicetype = powerdev_msg_type (&msg_c);

            char *clientname = "NUT";
            
            // look for a client 
            common_msg_t* retClient = select_client(url.c_str(), clientname);

            uint32_t client_id = 0;
            uint32_t msgid = common_msg_id (retClient);

            if ( msgid  == COMMON_MSG_FAIL )
                // the client was not found
                log_error("client with name='%s' was not found, message was ignored\n", clientname);
            else if ( msgid == COMMON_MSG_RETURN_CLIENT )
            {
                client_id = common_msg_rowid (retClient); // the client was found
           
                // look for a device
                // device is indicated by devicename and devicetype
                common_msg_t* retDevice = select_device(url.c_str(), devicetype, devicename);
            
                uint32_t device_id = 0;
                msgid = common_msg_id (retDevice);
            
                if ( msgid == COMMON_MSG_FAIL )
                {
                    // the device was not found, then insert new device
                    common_msg_t* newDevice = insert_device(url.c_str(), devicetype, devicename);
                
                    uint32_t newmsgid = common_msg_id (newDevice);

                    if ( newmsgid  == COMMON_MSG_FAIL )
                        // the device was not inserted
                        log_info("device with name='%s' and type='%s' was not inserted,"
                            "message was ignored\n", devicename, devicetype);
                    else if ( newmsgid == COMMON_MSG_DB_OK )
                        device_id = common_msg_rowid ( newDevice ); // the device was inserted
                    else
                        assert (false); // unknown response
                    common_msg_destroy (&newDevice);
                }
                else if ( msgid == COMMON_MSG_RETURN_DEVICE )
                    // the device was found
                    device_id = common_msg_rowid (retDevice);
                else
                    assert (false); // unknown response
                
                common_msg_destroy (&retDevice);
                 
                if (device_id != 0 ) // device was found or inserted
                {
                     // create blob information
                     zmsg_t *zmsg = powerdev_msg_encode_powerdev_status (
                         powerdev_msg_deviceid(&msg_c),
                         powerdev_msg_model(&msg_c),
                         powerdev_msg_manufacturer(&msg_c),
                         powerdev_msg_serial(&msg_c),
                         powerdev_msg_type(&msg_c),
                         powerdev_msg_status(&msg_c),
                         powerdev_msg_otherproperties(&msg_c)
                    );
                    assert (zmsg);
                    byte *encoded;
                    size_t infolen = zmsg_encode (zmsg, &encoded);
                    assert (encoded);
                    // inserting into client_info
                    common_msg_t* newClientInfo = insert_client_info
                                    (url.c_str(), device_id, client_id, encoded, infolen);
                    assert (newClientInfo);
                    msgid = common_msg_id (newClientInfo);
                    if ( msgid == COMMON_MSG_FAIL )
                        log_info("information about device name='%s' and type='%s' from the client='%s'"
                            "was not inserted into v_bios_client_info", devicename,devicetype,clientname);
                        // the info was not inserted
                    else if ( msgid == COMMON_MSG_DB_OK )
                    {
                        // some code for testing
                        // common_msg_t* nn = select_client_info(url.c_str(), common_msg_rowid(newClientInfo));
                        // assert (nn);
                        // zmsg_t* nnmsg = common_msg_msg(nn);
                        // assert (nnmsg);
                        // common_msg_t* newClientInfo1 = common_msg_decode (&nnmsg);
                        // assert (newClientInfo1);
                        // zchunk_t* ch = common_msg_info(newClientInfo1);
                        // assert(ch);
                        // size_t ss = zchunk_size(ch);
                        // zmsg_t* nm = zmsg_decode (zchunk_data(ch), ss);
                        // powerdev_msg_t* mm = powerdev_msg_decode (&nm);
                        // powerdev_msg_print(mm);
                        result = true;
                    }
                    else
                        assert (false); // unknown response
                    common_msg_destroy (&newClientInfo);
                    free (encoded);
                    zmsg_destroy (&zmsg);
                }
            }
            else
                assert (false); // unknown response
            common_msg_destroy (&retClient);
        }   // end case

    } // end switch

    return result;
}

} // namespace persist

