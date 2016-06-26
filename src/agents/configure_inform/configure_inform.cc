/*
Copyright (C) 2014-2015 Eaton

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

#include "configure_inform.h"

#include <malamute.h>
#include <stdexcept>

#include <bios_proto.h>
#include "str_defs.h"
#include "assets.h"
#include "dbpath.h"

static zhash_t*
s_map2zhash (const std::map<std::string, std::string>& m)
{
    zhash_t *ret = zhash_new ();
    zhash_autofree (ret);
    for (const auto& it : m) {
        zhash_insert (ret, ::strdup (it.first.c_str()), ::strdup (it.second.c_str()));
    }
    return ret;
}

void
    send_configure (
        const std::vector <std::pair<db_a_elmnt_t,persist::asset_operation>> &rows,
        const std::string &agent_name)
{
    mlm_client_t *client = mlm_client_new();
    
    if ( client == NULL ) {
        throw std::runtime_error(" mlm_client_new () failed.");
    }
    int r = mlm_client_connect (client, MLM_ENDPOINT, 1000, agent_name.c_str ());
    if ( r == -1 ) {
        mlm_client_destroy (&client);
        throw std::runtime_error(" mlm_client_connect () failed.");
    }

    r = mlm_client_set_producer (client, BIOS_PROTO_STREAM_ASSETS);
    if ( r == -1 ) {
        mlm_client_destroy (&client);
        throw std::runtime_error(" mlm_client_set_producer () failed.");
    }
    tntdb::Connection conn = tntdb::connectCached (url);
    for ( const  auto &oneRow : rows ) {
        
        char *s_priority, *s_parent;
        r = asprintf (&s_priority, "%u", (unsigned)  oneRow.first.priority);
        assert (r != -1);
        r = asprintf (&s_parent, "%lu", (long) oneRow.first.parent_id);
        assert (r != -1);

        char *subject;
        r = asprintf (&subject, "%s.%s@%s", persist::typeid_to_type (oneRow.first.type_id).c_str(), persist::subtypeid_to_subtype (oneRow.first.subtype_id).c_str(), oneRow.first.name.c_str());
        assert ( r != -1);

        // db_reply <db_web_basic_element_t>
        if (oneRow.first.parent_id != 0) { 
        }

        zhash_t *aux = zhash_new ();
        zhash_autofree (aux);
        zhash_insert (aux, "priority", (void*) s_priority);
        zhash_insert (aux, "type", (void*) persist::typeid_to_type (oneRow.first.type_id).c_str());
        zhash_insert (aux, "subtype", (void*) persist::subtypeid_to_subtype (oneRow.first.subtype_id).c_str());
        zhash_insert (aux, "parent", (void*) s_parent);
        zhash_insert (aux, "status", (void*) oneRow.first.status.c_str());
        if (oneRow.first.parent_id != 0) {
            auto parent_reply  = persist::select_asset_element_web_byId (conn, oneRow.first.parent_id);
            if (parent_reply.status != 1) {
                zhash_destroy (&aux);
                mlm_client_destroy (&client);
                throw std::runtime_error ("persist::select_asset_element_web_byId () failed.");
            }
            zhash_insert (aux, "parent_name", (void *) parent_reply.item.name.c_str ());
        }

        zhash_t *ext = s_map2zhash (oneRow.first.ext);

        zmsg_t *msg = bios_proto_encode_asset (
                aux,
                oneRow.first.name.c_str(),
                operation2str (oneRow.second).c_str(),
                ext);
        r = mlm_client_send (client, subject, &msg);
        zhash_destroy (&ext);
        zhash_destroy (&aux);
        if ( r != 0 ) {
            mlm_client_destroy (&client);
            throw std::runtime_error("mlm_client_send () failed.");
        }
    }
    zclock_sleep (500); // ensure that everything was send before we destroy the client
    mlm_client_destroy (&client);
}

void
    send_configure (
        db_a_elmnt_t row,
        persist::asset_operation action_type,
        const std::string &agent_name)
{
    send_configure(std::vector<std::pair<db_a_elmnt_t,persist::asset_operation>>{std::make_pair(row, action_type)}, agent_name);
}

void
    send_configure (
        const std::pair<db_a_elmnt_t, persist::asset_operation> &row,
        const std::string &agent_name)
{
    send_configure(std::vector<std::pair<db_a_elmnt_t,persist::asset_operation>>{row}, agent_name);
}
