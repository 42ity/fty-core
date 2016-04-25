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

#include <tntdb/connect.h>
#include <tntdb/error.h>
#include <time.h>
#include <biosproto.h>

#include "assetcrud.h"
#include "persistencelogic.h"
#include "log.h"
#include "dbpath.h"
#include "cleanup.h"
#include "utils.h"
#include "utils++.h"
#include "ymsg-asset.h"

namespace persist {

// used by src/agents/dbstore
void process_measurement(
        zmsg_t **msg_p,
        TopicCache& c,
        MultiRowCache& multi_row )
{
    if (!msg_p || !*msg_p) {
        return;
    }

    bios_proto_t *m = bios_proto_decode (msg_p);
    if ( !m ) {
        log_error("Can't decode the biosproto, ignore it");
        zmsg_destroy(msg_p);
        return;
    }

    tntdb::Connection conn;
    try {
        conn = tntdb::connectCached(url);
        conn.ping();
    } catch (const std::exception &e) {
        log_error("Can't connect to the database");
        zmsg_destroy(msg_p);
        return;
    }

    std::string db_topic = std::string (bios_proto_type (m)) + "@" + std::string(bios_proto_element_src (m));

    m_msrmnt_value_t value = 0;
    m_msrmnt_scale_t scale = 0;
    if (!strstr (bios_proto_value (m), ".")) {
        value = string_to_int64 (bios_proto_value (m));
        if (errno != 0) {
            errno = 0;
            bios_proto_destroy (&m);
            zmsg_destroy(msg_p);
            return;
        }
    }
    else {
        int8_t lscale = 0;
        int32_t integer = 0;
        if (!utils::math::stobiosf (bios_proto_value (m), integer, lscale)) {
            bios_proto_destroy (&m);
            zmsg_destroy(msg_p);
            return;
        }
        value = integer;
        scale = lscale;
    }

    // now represents time to live! bios_proto_time (m);
    // time is a time when message was received
    uint64_t _time = ::time(NULL);
    persist::insert_into_measurement(
            conn, db_topic.c_str(), value, scale, _time,
            bios_proto_unit (m), bios_proto_element_src (m), c, multi_row);
    bios_proto_destroy (&m);
    zmsg_destroy(msg_p);
}

void flush_measurement(MultiRowCache& multi_row ) {
    tntdb::Connection conn;
    try {
        conn = tntdb::connectCached(url);
        conn.ping();
    } catch (const std::exception &e) {
        log_error("Can't connect to the database");
        return;
    }
    persist::flush_measurement(conn,multi_row);
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
    if (streq(in_subj, "get_asset_extra") ) {
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
