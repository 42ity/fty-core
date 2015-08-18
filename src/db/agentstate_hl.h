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

/*! \file   agentstatehl.h
    \brief  High level API for storing agent state as binary entity
    \author Tomas Halman <TomasHalman@Eaton.com>
*/

#ifndef SRC_DB_AGENTSTATE_HL_H_
#define SRC_DB_AGENTSTATE_HL_H_

#include "db/agentstate.h"

int save_agent_info( tntdb::Connection& conn, const std::string& agent_name, const std::string& data);
int save_agent_info( const std::string& agent_name, const std::string& data);

std::string load_agent_info(tntdb::Connection& conn, const std::string& agent_name);
std::string load_agent_info(const std::string& agent_name);

#endif // SRC_DB_AGENTSTATE_HL_H_
