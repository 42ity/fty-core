/*
Copyright (C) 2015 Eaton

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

/*!
 \file   cm-agent-utils.h
 \brief  TODO
 \author Karol Hrdina <KarolHrdina@eaton.com>
*/
#ifndef SRC_AGENTS_COMPUTATION_CM_AGENT_UTILS_H__
#define SRC_AGENTS_COMPUTATION_CM_AGENT_UTILS_H__

#include "ymsg.h"
#include "bios_agent.h"

// TODO: doxygen
// original - ownership transfer
int
replyto_err
(bios_agent_t *agent, ymsg_t **original, const char *sender, const char *error_message, const char *subject);


#endif // SRC_AGENTS_COMPUTATION_CM_AGENT_UTILS_H__


