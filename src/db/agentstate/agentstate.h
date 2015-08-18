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

/*! \file   agentstate.h
    \brief  API for storing agent state as binary entity
    \author Alena Chernikava <AlenaChernikava@eaton.com>
*/

#ifndef SRC_DB_AGENTSTATE_AGENTSTATE_H_
#define SRC_DB_AGENTSTATE_AGENTSTATE_H_

#include <tntdb/connect.h>
int
    update_agent_info(
        tntdb::Connection &conn,
        const std::string &agent_name,
        const void        *data,
        size_t             size,
        uint16_t          &affected_rows
        );

int
    select_agent_info(
        tntdb::Connection  &conn,
        const std::string  &agent_name,
        void              **data,
        size_t             &size
        );

#endif // SRC_DB_AGENTSTATE_AGENTSTATE_H_
