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

#include <map>
#include <string>

#include "ymsg.h"
#include "bios_agent.h"

// TODO: doxygen, anonymous namespace
// original - ownership transfer
int
replyto_err
(bios_agent_t *agent, ymsg_t **original, const char *sender, const char *error_message, const char *subject);

int
process_db_measurement_json_to_map
(const char *json, std::map <int64_t, double>& samples, int64_t& start_timestamp, int64_t& end_timestamp, uint64_t& element_id, std::string& source, std::string& unit);

void
process_db_measurement_solve_left_margin
(std::map <int64_t, double>& samples, int64_t extended_start);

int
process_db_measurement_calculate 
(std::map <int64_t, double>& samples, int64_t start, int64_t end, const char *type, double& result);

int
process_db_measurement_calculate_arithmetic_mean
(std::map <int64_t, double>& samples, int64_t start, int64_t end, const char *type, double& result);

// -1 on error (begin > end)
int64_t
sample_weight (int64_t begin, int64_t end);
#endif // SRC_AGENTS_COMPUTATION_CM_AGENT_UTILS_H__


