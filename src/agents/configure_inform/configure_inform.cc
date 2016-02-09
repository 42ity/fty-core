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

#include <biosproto.h>

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

        zhash_insert (ext2, "type",
                (void*) persist::typeid_to_type (oneRow.first.type_id).c_str ());
        zhash_insert (ext2, "subtype",
                (void*) persist::subtypeid_to_subtype (oneRow.first.subtype_id).c_str ());
        //TODO: what to do with parent_id?
        zhash_insert (ext2, "status",
                (void*) oneRow.first.status.c_str());
        zhash_insert (ext2, "priority",
                (void*) std::to_string (oneRow.first.priority).c_str ());

        zmsg_t *zmsg = bios_proto_encode_asset (
                NULL,
                oneRow.first.name.c_str(),
                "TODO",
                ext2);
        //TODO: maybe cleanup the zhash_t
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
