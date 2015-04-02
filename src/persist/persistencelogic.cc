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
        Michal Hrusecky <MichalHrusecky@eaton.com>
*/

/* TODO
 - Ip address is being stored as string at the moment; Store it as two byte arrays (hi, lo).
*/

#include <assert.h>
#include <czmq.h>
#include <tntdb/connect.h>
#include <tntdb/error.h>
#include <time.h>

#include <zmq.h>
#include <czmq.h>


#include "common_msg.h"

#include "defs.h"
#include "bios_agent.h"
#include "preproc.h"
#include "assetcrud.h"
#include "cidr.h"
#include "persistence.h"
#include "persistencelogic.h"
#include "monitor.h"
#include "log.h"
#include "dbpath.h"
#include "measurement.h"
#include "agents.h"

#define NETHISTORY_AUTO_CMD     'a'
#define NETHISTORY_MAN_CMD      'm'
#define NETHISTORY_EXCL_CMD     'e'

namespace persist {

void process_measurement(UNUSED_PARAM const std::string &topic, zmsg_t **msg) {
    log_debug("Processing measurement");
    ymsg_t *ymsg = ymsg_decode(msg);
    if(ymsg == NULL) {
        log_error("Can't decode the ymsg");
        return;
    }

    tntdb::Connection conn;
    try {
        conn = tntdb::connectCached(url);
        conn.ping();
    } catch (const std::exception &e) {
        log_error("Can't connect to the database");
        ymsg_destroy(&ymsg);
        return;
    }

    int64_t tme = 0;
    char *device_name = NULL;
    char *quantity    = NULL;   // TODO: THA: what does this parameter mean?
    char *units       = NULL;
    m_msrmnt_value_t value = 0;
    int32_t scale = -1;

    int rv = bios_measurement_decode (&ymsg, &device_name, &quantity, 
                                      &units, &value, &scale, &tme);
    if ( rv != 0 ) {
        log_error("Can't decode the ymsg, ignore it");
        return;
    }

    // TODO: MVY, why this is here???
    if(tme < 1)
        tme = ::time(NULL);

    std::string db_topic = std::string (quantity) + "@" + device_name; 
    time_t _time = (time_t) tme;
    persist::insert_into_measurement(
            conn, db_topic.c_str(), value, (m_msrmnt_scale_t) scale, _time, units, device_name);
}

zmsg_t* asset_msg_process(zmsg_t **msg) {
    log_debug("Processing asset message in persistence layer");
    zmsg_t *result = NULL;

    asset_msg_t *amsg = asset_msg_decode(msg);
    if(amsg == NULL) {
        log_warning ("Malformed asset message received!");
        return common_msg_encode_fail(BAD_INPUT, BAD_INPUT_WRONG_INPUT,
                                        "Malformed asset message message received!", NULL);
    }

    int msg_id = asset_msg_id (amsg);
    
    switch (msg_id) {

        case ASSET_MSG_GET_ELEMENT: {
            // datacenter|room|row|rack|group|device
            result = get_asset_element(url.c_str(), amsg);
            break;
        }
        case ASSET_MSG_UPDATE_ELEMENT:
        case ASSET_MSG_INSERT_ELEMENT:
        case ASSET_MSG_DELETE_ELEMENT: {
            //not implemented yet
            break;
        }
        case ASSET_MSG_GET_ELEMENTS: {
            // datacenters|rooms|rows|racks|groups
            result = get_asset_elements(url.c_str(), amsg);
            break;
        }
        case ASSET_MSG_ELEMENT:
        case ASSET_MSG_RETURN_ELEMENT:
        case ASSET_MSG_OK:
        case ASSET_MSG_FAIL:
        case ASSET_MSG_RETURN_ELEMENTS:
        default: {
            log_warning ("Wrong asset message (id %d) received!", msg_id);
            result = common_msg_encode_fail(BAD_INPUT, BAD_INPUT_WRONG_INPUT,
                                            "Wrong asset message message received!", NULL);

            break;
        }
    }

    asset_msg_destroy(&amsg);
    return result;
};


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
    log_info ("start");

    zmsg_t *msg2;

    //XXX: this is ugly, however we need to distinguish between types of messages
    //     zccp will solve that by DIRECT message
    zmsg_print(msg);
    msg2 = zmsg_dup(msg);
    zmsg_pop(msg2);
    assert(msg2);
    powerdev_msg_t *powerdev_msg = powerdev_msg_decode(&msg2);
    if (powerdev_msg) {
        //TODO: check the log level!
        powerdev_msg_print(powerdev_msg);
        return powerdev_msg_process(url, *powerdev_msg);
    }
    msg2 = zmsg_dup(msg);
    zmsg_pop(msg2);
    assert(msg2);
    common_msg_t *common_msg = common_msg_decode(&msg2);
    if (common_msg) {
        //TODO: check the log level!
        common_msg_print(common_msg);
        return common_msg_process(url, *common_msg);
    }

    log_error("unsupported message type, skipped!");
    log_info ("end");
    return false;
}

/*!
\brief stores the supplied nmap message

\author Karol Hrdina <karolhrdina@eaton.com>
\todo better exception handling granularity
*/

zmsg_t* nmap_msg_process(zmsg_t **msg) {
    nmap_msg_t *nmsg = nmap_msg_decode(msg);
    zmsg_t *ret = NULL;

    if(nmsg == NULL) {
        log_warning ("Malformed nmap message received!");
        return common_msg_encode_fail(BAD_INPUT, BAD_INPUT_WRONG_INPUT,
                                        "Malformed nmap message message received!", NULL);
    }

    int msg_id = nmap_msg_id(nmsg);
    switch (msg_id) {
        case NMAP_MSG_LIST_SCAN:
        {
            // data checks
            const char *ip = nmap_msg_addr (nmsg);
            if (ip == NULL) {
                log_error ("empty 'addr' field of NMAP_MSG_LIST_SCAN received");
                ret =  common_msg_encode_fail(BAD_INPUT, BAD_INPUT_WRONG_INPUT,
                        "empty 'addr' field of NMAP_MSG_LIST_SCAN received", NULL);
                break;
            }

            // establish connection && execute
            try
            {
                tntdb::Connection connection = tntdb::connectCached (url);
                tntdb::Statement st = connection.prepare(
                    "INSERT INTO t_bios_discovered_ip (timestamp, ip) VALUES (UTC_TIMESTAMP(), :v1)");
                st.setString("v1", ip).execute();
            }
            catch (const std::exception& e)
            {
                log_error ("exception caught: %s", e.what ());
                ret =  common_msg_encode_fail(DB_ERR, DB_ERROR_INTERNAL,
                        e.what(), NULL);
            }
            catch (...)
            {
                log_error ("tntdb::connectCached(%s) failed: Unknown exception caught.", url.c_str());
                ret =  common_msg_encode_fail(DB_ERR, DB_ERROR_INTERNAL,
                        "NMAP: Unknown exception", NULL);
            }
            break;
        }

        case NMAP_MSG_DEV_SCAN:
        {
            log_info ("NMAP_MSG_DEV_SCAN not implemented at the moment.");
            ret =  common_msg_encode_fail(DB_ERR, DB_ERROR_NOTIMPLEMENTED,
                        "NMAP_MSG_DEV_SCAN not implemented", NULL);
            break;
        }

        default: {
            log_warning ("Unexpected message type received; message id = '%d'", msg_id);
            ret = common_msg_encode_fail(BAD_INPUT, BAD_INPUT_WRONG_INPUT,
                                  "Unexpected message type received!", NULL);
            break;
        }
    }
    nmap_msg_destroy(&nmsg);
    if(ret != NULL)
        return ret;
    return common_msg_encode_db_ok(0, NULL);
}

zmsg_t* netdisc_msg_process(zmsg_t** msg) {

    // cast away the const - zproto generated methods dont' have const
    netdisc_msg_t* msg_nc = netdisc_msg_decode(msg);
    if(msg_nc == NULL) {
        log_warning ("Malformed netdisc message received!");
        return common_msg_encode_fail(BAD_INPUT, BAD_INPUT_WRONG_INPUT,
                                        "Malformed netdisc message message received!", NULL);
    }

    int msg_id          = netdisc_msg_id (msg_nc);
    const char *name    = netdisc_msg_name (msg_nc);
    const int ipver     = static_cast<int>(netdisc_msg_ipver (msg_nc));
    const char *ipaddr  = netdisc_msg_ipaddr (msg_nc);
    int prefixlen       = static_cast<int>(netdisc_msg_prefixlen (msg_nc));
    std::string mac (netdisc_msg_mac (msg_nc));
    char command        = nethistory_id_cmd (netdisc_msg_id (msg_nc));
    if((command == 0) || (ipver!=0 && ipver!=1)) {
        log_warning ("Malformed insides of netdisc message received!");
        return common_msg_encode_fail(BAD_INPUT, BAD_INPUT_WRONG_INPUT,
                              "Malformed insides of netdisc message message received!", NULL);
    }
    shared::CIDRAddress address (ipaddr, prefixlen);

    unsigned int rows_affected = 0;

    persist::NetHistory nethistory(url);
    nethistory.setAddress(address);
    nethistory.setCommand(command);
    nethistory.setName(name);
    nethistory.setMac(mac);

    int id_unique = nethistory.checkUnique();
    if(id_unique == -1) {
        log_warning("Nethistory ID is not unique");
        return common_msg_encode_fail(BAD_INPUT, BAD_INPUT_WRONG_INPUT,
                              "Nethistory ID is not unique!", NULL);
    }

    switch (msg_id) {
        case NETDISC_MSG_AUTO_ADD: {
            rows_affected = nethistory.dbsave();
            break;
        }
        case NETDISC_MSG_AUTO_DEL: {
            rows_affected = nethistory.deleteById (id_unique);
            break;
        }
        case NETDISC_MSG_MAN_ADD: {
            rows_affected = nethistory.dbsave();
            break;
        }
        case NETDISC_MSG_MAN_DEL: {
            rows_affected = nethistory.deleteById(id_unique);
            break;
        }
        case NETDISC_MSG_EXCL_ADD: {
            rows_affected = nethistory.dbsave();
            break;
        }
        case NETDISC_MSG_EXCL_DEL: {
            rows_affected = nethistory.deleteById(id_unique);
            break;
        }
        default: {
            log_warning ("Unexpected message type received; message id = '%d'", msg_id);
            return common_msg_encode_fail(BAD_INPUT, BAD_INPUT_WRONG_INPUT,
                                  "Unexpected message type received!", NULL);
            break;
        }
    }
    if(rows_affected != 1) {
        log_warning ("Unexpected number of rows '%u' affected", rows_affected);
    }
    return common_msg_encode_db_ok(nethistory.getId(), NULL);
};

// * \brief processes the powerdev_msg message
/**
 * ASSUMPTION: different drivers should provide a consistent information:
 * DriverA reads voltageA as key="voltage", subkey = 1;
 * DriverB MUST read voltageA as key="voltage", subkey = 1; and
 * NOT key = "voltage", subkey = 2;
 *
 * *if it is not true, HERE WE DON'T CARE. It is a mistake in driver!!!!!!
 *
 * from, about, key , subkey, value
 *
 * process common_msg: new_measurement
 *                      get_measurement
 * process common_msg: insert_metadata
 *
 * TODO: asspumption: this metadata are not supposed to be used now.
 *
 *
 * not measurable information - some basic info, or driver configuration should be send to t_bios_client_info
 *
 * \brief processes the common_msg
 *
 * \param url - a connection to the database
 * \param msg - a message to be processed
 *
 * \return  true  - if message was processed successfully
 *          false - if message was ignored
 */
bool
common_msg_process(UNUSED_PARAM const std::string& url, const common_msg_t& msg)
{
    bool result = false;

    // cast away the const - zproto generated methods don't have const
    common_msg_t& msg_nc = const_cast<common_msg_t&>(msg);

    int msg_id = common_msg_id (&msg_nc);

    switch (msg_id) {

        case COMMON_MSG_NEW_MEASUREMENT:
        {
            //TODO
            log_error ("this should never happen");
  //          bool r = insert_new_measurement(url.c_str(), &msg_nc);
//FIXME: JIM: 20150109 - if there was an error inserting the value, what should
//we return? it was not "ignored" but did not end up in database either...
            result = true;
        //if (!r)
        //    log_warning ("Did not succeed inserting new measurement; message id = '%d'", msg_id);
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
            log_warning ("Unexpected message type received; message id = '%d'", msg_id);
            break;
        }

    }
    return result;
};

/*
bool insert_new_measurement(const char* url, common_msg_t* msg)
{
    const char* devicename  = common_msg_device_name (msg);
    const char* devicetype  = common_msg_device_type (msg);
    const char* clientname  = common_msg_client_name (msg);
    m_msrmnt_tp_id_t   mt_id  = common_msg_mt_id (msg);
    m_msrmnt_sbtp_id_t mts_id = common_msg_mts_id (msg);
    m_msrmnt_value_t   value  = common_msg_value (msg);

    bool result = false;

    // look for a client
    // TODO: May be it would be better to select all clients from database, to save time
    // and to minimize the amount of requests.
    // If new client added to the system, then at first it should register!!!!!
    // and during the registration, the list of actual clients could be updated
    common_msg_t* retClient = select_client(url, clientname);

    int msgid = common_msg_id (retClient);

    if ( msgid  == COMMON_MSG_FAIL )
        // the client was not found
        log_error("client with name='%s' was not found, message was ignored", clientname);
    else if ( msgid == COMMON_MSG_RETURN_CLIENT )
    {
        m_clnt_id_t client_id = common_msg_rowid (retClient); // the client was found

        // look for a device
        // device is indicated by devicename and devicetype
        // If such device is not in the system, then ignore the message
        // because: ASSUMPTION: all devices are already inserted into the system by administrator
        common_msg_t* retDevice = select_device(url, devicetype, devicename);

        msgid = common_msg_id (retDevice);

        if ( msgid == COMMON_MSG_FAIL )
            // the device was not found
            log_error("device with name='%s' was not found, message was ignored", devicename);
        else if ( msgid == COMMON_MSG_RETURN_DEVICE )
        {   // the device was found
            m_dvc_id_t device_id = common_msg_rowid (retDevice);

            common_msg_t* imeasurement = insert_measurement(url, client_id, device_id, mt_id,
                                mts_id, value);
            assert ( imeasurement );

            msgid = common_msg_id (imeasurement);
            if ( msgid == COMMON_MSG_FAIL )
                log_info("information about device name='%s' and type='%s' from the client='%s'"
                        "was not inserted into v_bios_client_measurement", devicename,devicetype,clientname);
                // the info was not inserted
            else if ( msgid == COMMON_MSG_DB_OK )
                result = true;
            else
                assert (false); // unknown response
            // now we don't want to return this message, so destroy it
            common_msg_destroy (&imeasurement);
        }
        else
            assert (false); //unknown response
        common_msg_destroy (&retDevice);
    }
    else
        assert (false); // unknown response

    common_msg_destroy (&retClient);
    return result;
};*/

bool insert_new_client_info(const char* url, common_msg_t* msg)
{
    m_clnt_id_t client_id = common_msg_client_id (msg);
    m_dvc_id_t device_id UNUSED_PARAM = common_msg_device_id (msg);
    zchunk_t* info = common_msg_info (msg);
    time_t date UNUSED_PARAM = common_msg_date (msg);

    bool result = false;

    //FIXME: we now support is == 5 == UI_properties
    assert (client_id == UI_PROPERTIES_CLIENT_ID);

    common_msg_t *reply = update_client_info(url, client_id, &info);
    int msgid = common_msg_id (reply);

    if ( msgid  == COMMON_MSG_FAIL )
        // the client info was not found
        log_error("client info for UI/properties not found, message was ignored");
    else if ( msgid == COMMON_MSG_DB_OK )
        result = true;
    else
        log_error("Invalid reply for update_client_info, client_id: %d, message was ignored", client_id);

    common_msg_destroy (&reply);

    return result;
};


/**
 * \brief processes the powerdev_msg
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

            const char* clientname = "NUT";

            // look for a client
            common_msg_t* retClient = select_client(url.c_str(), clientname);

            m_clnt_id_t client_id = 0;
            int msgid = common_msg_id (retClient);

            if ( msgid  == COMMON_MSG_FAIL )
                // the client was not found
                log_error("client with name='%s' was not found, message was ignored", clientname);
            else if ( msgid == COMMON_MSG_RETURN_CLIENT )
            {
                client_id = common_msg_rowid (retClient); // the client was found

                // look for a device
                // device is indicated by devicename and devicetype
                common_msg_t* retDevice = select_device(url.c_str(), devicetype, devicename);

                m_dvc_id_t device_id = 0;
                msgid = common_msg_id (retDevice);

                if ( msgid == COMMON_MSG_FAIL )
                {
                    // the device was not found, then insert new device
                    common_msg_t* newDevice = insert_device(url.c_str(), devicetype, devicename);

                    int newmsgid = common_msg_id (newDevice);

                    if ( newmsgid  == COMMON_MSG_FAIL )
                        // the device was not inserted
                        log_info("device with name='%s' and type='%s' was not inserted,"
                            "message was ignored", devicename, devicetype);
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

/**
 * \brief Processes message of type common_msg_t
 *
 * Broken down processing of generic database zmsg_t, this time common message
 * case.
 */
zmsg_t* common_msg_process(zmsg_t **msg) {
    common_msg_t *cmsg = common_msg_decode(msg);
    if(cmsg == NULL) {
        log_warning("Malformed common message!");
        return common_msg_encode_fail(BAD_INPUT, BAD_INPUT_WRONG_INPUT,
                                  "Malformed common message!", NULL);
    }
    zmsg_t *ret = NULL;
    int msg_id = common_msg_id (cmsg);
    switch (msg_id) {
    case COMMON_MSG_NEW_MEASUREMENT: {
        //TODO
    //    insert_new_measurement(url.c_str(), cmsg);
        log_error ("old protocol, this should never happen");
        break;
    }
    case COMMON_MSG_INSERT_DEVICE: {
        zmsg_t *tmpz = common_msg_msg(cmsg);
        if(tmpz != NULL) {
        common_msg_t *tmpc = common_msg_decode(&tmpz);
        if(tmpc != NULL) {
        common_msg_t *retc = insert_device(url.c_str(),
                           common_msg_devicetype_id(tmpc),
                           common_msg_name(tmpc));
        common_msg_destroy(&tmpc);
        ret = common_msg_encode(&retc);
        }}
        break;
    }
    case COMMON_MSG_INSERT_CLIENT: {
        zmsg_t *tmpz = common_msg_msg(cmsg);
        if(tmpz != NULL) {
        common_msg_t *tmpc = common_msg_decode(&tmpz);
        if(tmpc != NULL) {
        common_msg_t *retc = insert_client(url.c_str(),
                           common_msg_name(tmpc));
        common_msg_destroy(&tmpc);
        ret = common_msg_encode(&retc);
        }}
        break;
    }
    case COMMON_MSG_GET_MEASURE_TYPE_I:
    case COMMON_MSG_GET_MEASURE_TYPE_S:
    case COMMON_MSG_GET_MEASURE_SUBTYPE_I:
    case COMMON_MSG_GET_MEASURE_SUBTYPE_S: {
        log_error ("old get measure are not supported, ignore them");
        break;
    }
    default: {
        log_warning("Got wrong common message!");
        common_msg_encode_fail(BAD_INPUT, BAD_INPUT_WRONG_INPUT,
                   "Wrong common message!", NULL);

        break;
    }
    }
    common_msg_destroy(&cmsg);
    return ret;
}

/**
 * \brief Processes message of type ymsg_t
 *
 * Processing of generic ymsg_t and breaking it into particular processing
 * functions for ymsg.
 */
void process_ymsg(ymsg_t* out, char** out_subj, ymsg_t* in, const char* in_subj) {
    if(streq(in_subj, "get_measurements")) {
        persist::get_measurements(out, out_subj, in, in_subj);
        return;
    }
}

/**
 * \brief Initial routing of messages
 *
 * Processing of generic zmsg_t, breaking it into particular processing
 * functions.
 */
zmsg_t* process_message(zmsg_t** msg) {
    if((msg == NULL) || (*msg == NULL)) return NULL;
    if(is_common_msg(*msg)) {
        return common_msg_process(msg);
    } else if(is_asset_msg(*msg)) {
        return asset_msg_process(msg);
    } else if(is_netdisc_msg(*msg)) {
        return netdisc_msg_process(msg);
    } else if(is_nmap_msg(*msg)) {
        return nmap_msg_process(msg);
    } else {
        log_warning("Got wrong message!");
        return common_msg_encode_fail(BAD_INPUT, BAD_INPUT_WRONG_INPUT,
                                  "Wrong message received!", NULL);
    }
}


// should process destroy ymsg?
//
// TODO do we want to have void here?
void process_inventory (ymsg_t **msg)
{
    // TODO need to analyse the repeat key
    LOG_START;
    if ( msg == NULL ) {
        log_error ("NULL pointer to ymsg");
        return;
    }

    char *device_name    = NULL;
    char *module_name    = NULL;
    zhash_t    *ext_attributes = NULL;

    int rv = bios_inventory_decode 
                (msg, &device_name, &ext_attributes, &module_name);
    
    if  ( rv != 0 )
    {
        // TODO: should we log whole message ?
        log_error ("ignore msg: Malformed content of request message");
        return;
    }

    tntdb::Connection conn;
    try {
        conn = tntdb::connectCached(url);
        conn.ping();
        process_insert_inventory (conn, device_name, ext_attributes);

        // RESULT is unimportant, because pub/sub.
        //
        // for REQ/REP write other wrapper
        // TODO should be analysed result
    } catch (const std::exception &e) {
        log_error("ignore msg: Can't connect to the database");
        return;
    }

      
    
    // TODO result        
}

} // namespace persist
