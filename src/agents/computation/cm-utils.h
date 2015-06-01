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
 \file   cm-utils.h
 \brief  Header file for utility functions and helpers for computation module
 \author Karol Hrdina <KarolHrdina@eaton.com>
*/
#ifndef SRC_AGENTS_COMPUTATION_CM_UTILS_H__
#define SRC_AGENTS_COMPUTATION_CM_UTILS_H__

#include <map>
#include <string>

#include "ymsg.h"
#include "bios_agent.h"

namespace computation {

namespace web {

/*
 \brief Return weight of a sample at \a begin timestamp
 \param[in] timestamp of the sample
 \param[in] timestamp of the next sample
 \return weight of the sample at \a begin timestamp, -1 if \a begin >= \a end
*/ 
int64_t
sample_weight (int64_t begin, int64_t end);

void
solve_left_margin
(std::map <int64_t, double>& samples, int64_t extended_start);

int
calculate 
(std::map <int64_t, double>& samples, int64_t start, int64_t end, const char *type, double& result);

int
calculate_arithmetic_mean
(std::map <int64_t, double>& samples, int64_t start, int64_t end, double& result);

/*!
 \brief Check completeness of request for averages and decide whether to compute any missing averages from the set.

 When averages are request, there may be items missing from the set.
 For certain conditions we don't need to ask for sampled data and try to re-calculate these averages, for other conditions we need to.
 The consensus is: persistece returns timestamp of absolute last stored average of given topic. Anything before that is considered (and should be)
 non-recomputable. 
 This functions checks these conditions and returns whether we should attempt to compute the missing averages.

 \param[in] last_container_timestamp    timestamp of the last item in the container of averages returned by persistence
 \param[in] last_average_timestamp      timestamp of the last average stored in persistence
 \param[in] end_timestamp               end timestamp of the request interval
 \param[in] step                        step of the requsted average
 \param[out] new_start                  timestamp of the first average (of the given \a step) that is missing
 \return 1,     set is complete, \a new_start is not altered
         0,     set is not complete, \a new_start is set
         -1,    error, supplied values do not make sense (e.g.: last container timestamp < last average timestamp <= end timestamp)
*/
int
check_completeness
(int64_t last_container_timestamp, int64_t last_average_timestamp, int64_t end_timestamp, const char *step, int64_t& new_start);

/*
 \brief Wrapper function for persist::get_measurements_averages

 Performs the call and takes proper care of the return values

 \param[in]  element_id         id of the requested element
 \param[in]  source             requested source
 \param[in]  type               requested type
 \param[in]  step               requested step
 \param[in]  start_timestamp    start timestamp of the requested period
 \param[in]  end_timestamp      end timestamp of the requested period
 \param[out] averages           std::map to be filled with requested averages 
 \param[out] unit               unit of requested measurements 
 \param[out] last_average_timestamp     timestamp of the last average (of given topic) stored in persistence
 \param[out] message_out         

 \return 0, error; message_out is set
         1, success; ..
 requested source
 
*/
int
request_averages
(int64_t element_id, const char *source, const char *type, const char *step, int64_t start_timestamp, int64_t end_timestamp,
 std::map<int64_t, double>& averages, std::string& unit, int64_t& last_average_timestamp, ymsg_t *message_out);

int
request_sampled
(int64_t element_id, const char *topic, int64_t start_timestamp, int64_t end_timestamp,
 std::map<int64_t, double>& samples, std::string& unit, ymsg_t *message_out);

} // namespace computation::web

} // namespace computation

#endif // SRC_AGENTS_COMPUTATION_CM_UTILS_H__

