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

#include <stdexcept>

#include "bios_agent.h"
#include "cleanup.h"
#include "str_defs.h"
#include "agents.h"
#include "db/assets.h"
#include "dbpath.h"
#include "asset_types.h"

#include <biosproto.h>

static void
    s_parent_name (a_elmnt_id_t id, std::string &parent_name)
{
    try {
        tntdb::Connection conn = tntdb::connectCached (url);
        auto reply = persist::select_asset_element_web_byId (conn, id);
        if (reply.status == 0)
            parent_name = "";
        else
            parent_name = reply.item.name;
    }
    catch (const std::exception &e)
    {
        log_error ("fail to connect to DB: %s", e.what ());
    }
}

static const char*
    s_operation (persist::asset_operation o)
{
    switch (o) {
        case persist::asset_operation::INSERT:
            return "INSERT";
        case persist::asset_operation::DELETE:
            return "DELETE";
        case persist::asset_operation::UPDATE:
            return "UPDATE";
        case persist::asset_operation::GET:
            return "GET";
        case persist::asset_operation::RETIRE:
            return "RETIRE";
    }
    return "I'm making gcc happy!"; //make gcc happy
}

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
    bios_agent_t *agent = bios_agent_new (MLM_ENDPOINT, agent_name.c_str ());
    if ( agent == NULL )
        throw std::runtime_error(" bios_agent_new () failed.");

    // Hack for the new protocols
    mlm_client_t *client = mlm_client_new ();
    if (!client) {
        bios_agent_destroy (&agent);
        throw std::runtime_error(" mlm_client_new () failed.");
    }

    int r = mlm_client_connect (client, MLM_ENDPOINT, 5000, agent_name.c_str ());
    if (r == -1) {
        bios_agent_destroy (&agent);
        mlm_client_destroy (&client);
        throw std::runtime_error("mlm_client_connect () failed.");
    }
    bios_agent_set_producer (agent, bios_get_stream_main());
    mlm_client_set_producer (client, "ASSETS");

    for ( auto &oneRow : rows )
    {
        zhash_t *ext = s_map2zhash (oneRow.first.ext);
        zhash_t *ext2 = zhash_dup (ext);

        ymsg_t *msg = bios_asset_extra_encode (oneRow.first.name.c_str(), &ext, oneRow.first.type_id,
		oneRow.first.subtype_id, oneRow.first.parent_id, oneRow.first.status.c_str(),
		oneRow.first.priority, static_cast<int8_t>(oneRow.second));

        zhash_t *aux = zhash_new ();
        zhash_insert (aux, "type",
                (void*) persist::typeid_to_type (oneRow.first.type_id).c_str ());
        zhash_insert (aux, "subtype",
                (void*) persist::subtypeid_to_subtype (oneRow.first.subtype_id).c_str ());
        zhash_insert (aux, "status",
                (void*) oneRow.first.status.c_str());
        zhash_insert (aux, "priority",
                (void*) std::to_string (oneRow.first.priority).c_str ());

        std::string parent_name;
        s_parent_name (oneRow.first.id, parent_name);
        zhash_insert (aux, "parent",
                (void*) parent_name.c_str ());

        zmsg_t *zmsg = bios_proto_encode_asset (
                aux,
                oneRow.first.name.c_str(),
                s_operation (oneRow.second),
                ext2);
        zhash_destroy (&aux);
        zhash_destroy (&ext2);

        if ( msg == NULL || zmsg == NULL)
        {
            bios_agent_destroy (&agent);
            mlm_client_destroy (&client);
            throw std::runtime_error("bios_asset_encode () failed.");
        }

        const std::string topic = "configure@" + oneRow.first.name;
        int rv = bios_agent_send (agent, topic.c_str(), &msg);
        if ( rv != 0 )
        {
            zmsg_destroy (&zmsg);
            bios_agent_destroy (&agent);
            mlm_client_destroy (&client);
            throw std::runtime_error("bios_agent_send () failed.");
        }

        std::string subject = "@" + oneRow.first.name;
        rv = mlm_client_send (client, subject.c_str (), &zmsg);
        if (rv == -1)
        {
            bios_agent_destroy (&agent);
            mlm_client_destroy (&client);
            throw std::runtime_error("bios_agent_send () failed.");
        }

    }
    bios_agent_destroy (&agent);
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
