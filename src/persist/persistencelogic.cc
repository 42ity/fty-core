/*
Copyright (C) 2014 Eaton

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
#include "persistencelogic.h"
#include "monitor.h"
#include "log.h"
#include "dbpath.h"
#include "measurement.h"
#include "db/alerts.h"
#include "agents.h"
#include "cleanup.h"
#include "utils.h"
#include "ymsg-asset.h"

#define NETHISTORY_AUTO_CMD     'a'
#define NETHISTORY_MAN_CMD      'm'
#define NETHISTORY_EXCL_CMD     'e'

namespace persist {

// used by src/agents/dbstore
void process_measurement(UNUSED_PARAM const std::string &topic, ymsg_t **ymsg, TopicCache& c) {
    log_debug("Processing measurement");
    int64_t tme = 0;
    _scoped_char *device_name = NULL;
    _scoped_char *quantity    = NULL;   // TODO: THA: what does this parameter mean?
    _scoped_char *units       = NULL;
    m_msrmnt_value_t value = 0;
    int32_t scale = -1;
    int rv;
    std::string db_topic;
    time_t _time;

    tntdb::Connection conn;
    try {
        conn = tntdb::connectCached(url);
        conn.ping();
    } catch (const std::exception &e) {
        log_error("Can't connect to the database");
        goto free_mem_toto;
    }

    rv = bios_measurement_decode (ymsg, &device_name, &quantity, 
                                      &units, &value, &scale, &tme);
    if ( rv != 0 ) {
        log_error("Can't decode the ymsg, ignore it");
        goto free_mem_toto;
    }

    if(tme < 1)
        tme = ::time(NULL);

    db_topic = std::string (quantity) + "@" + device_name; 
    _time = (time_t) tme;
    persist::insert_into_measurement(
            conn, db_topic.c_str(), value, (m_msrmnt_scale_t) scale, _time, units, device_name, c);
free_mem_toto:
    //free resources
    if(*ymsg)
        ymsg_destroy(ymsg);
    FREE0 (device_name)
    FREE0 (quantity)
    FREE0 (units)
}


/**
 * \brief Processes message of type ymsg_t delivered as MAILBOX DELIVER
 *
 * Processing of generic ymsg_t and breaking it into particular processing
 * functions for ymsg.
 */
// used by src/agents/dbstore
void process_mailbox_deliver(ymsg_t** out, char** out_subj, ymsg_t* in, const char* in_subj) {
    if (!in_subj)
        return;

    if (strncmp(in_subj, "get_measurements", 17) == 0) {
        persist::get_measurements(out, out_subj, in, in_subj);
        return;
    }
    else if (strncmp(in_subj, "alert.", 6) == 0 ) {
        persist::process_alert(out, out_subj, in, in_subj);
        return;
    }
    else if (streq(in_subj, "get_asset_element") ) {
        persist::process_get_asset(out, out_subj, in, in_subj);
        return;
    }
    else if (streq(in_subj, "get_asset_extra") ) {
        persist::process_get_asset_extra (out, out_subj, in, in_subj);
        return;
    }
    else {
        log_debug("Unknown subject '%s', skipping", in_subj);
    }
}

// should process destroy ymsg?
//
// TODO do we want to have void here?
// used by src/agents/inventory
void process_inventory (ymsg_t **msg)
{
    // TODO need to analyse the repeat key
    LOG_START;
    if ( msg == NULL ) {
        log_error ("NULL pointer to ymsg");
        return;
    }

    _scoped_char *device_name    = NULL;
    _scoped_char *module_name    = NULL;
    _scoped_zhash_t    *ext_attributes = NULL;

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
