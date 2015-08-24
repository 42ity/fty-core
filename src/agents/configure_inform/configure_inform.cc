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

void
    send_configure (
        std::vector<db_a_elmnt_t> rows,
        uint8_t action_type,
        const std::string &agent_name)
{
    bios_agent_t *agent = bios_agent_new (MLM_ENDPOINT, agent_name.c_str ());
    if ( agent == NULL )
        throw std::runtime_error(" bios_agent_new () failed.");

    bios_agent_set_producer (agent, bios_get_stream_main());
    for ( auto &oneRow : rows )
    {
        ymsg_t *msg = bios_asset_encode (oneRow.name.c_str(), oneRow.type_id,
		oneRow.subtype_id, oneRow.parent_id, oneRow.status.c_str(),
		oneRow.priority, action_type);
        if ( msg == NULL )
        {
            bios_agent_destroy (&agent);
            throw std::runtime_error("bios_asset_encode () failed.");
        }
        const std::string topic = "configure@" + oneRow.name;
        int rv = bios_agent_send (agent, topic.c_str(), &msg);
        if ( rv != 0 )
        {
            bios_agent_destroy (&agent);
            throw std::runtime_error("bios_agent_send () failed.");
        }
    }
    bios_agent_destroy (&agent);
}

void
    send_configure (
        db_a_elmnt_t row,
        uint8_t action_type,
        const std::string &agent_name)
{
    send_configure(std::vector<db_a_elmnt_t>{row},action_type, agent_name);
}
