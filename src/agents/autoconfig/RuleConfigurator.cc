/* 
Copyright (C) 2014 - 2015 Eaton
 
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

/*!
 \file   RuleConfigurator.cc
 \brief  todo 
 \author Tomas Halman <TomasHalman@Eaton.com>
*/

#include "log.h"
#include "str_defs.h"
#include "utils_web.h"

#include "RuleConfigurator.h"

using namespace utils::json;

bool RuleConfigurator::sendNewRule (const std::string& rule, mlm_client_t *client)
{
    if (!client)
        return false;
    zmsg_t *message = zmsg_new ();
    zmsg_addstr (message, "ADD");
    zmsg_addstr (message, rule.c_str());
    if (mlm_client_sendto (client, BIOS_AGENT_NAME_ALERT_AGENT, "rfc-evaluator-rules", NULL, 5000, &message) != 0) {
        log_error ("mlm_client_sendto (address = '%s', subject = '%s', timeout = '5000') failed.",
                BIOS_AGENT_NAME_ALERT_AGENT, "rfc-evaluator-rules");
        return false;
    }
    return true;
}

std::string RuleConfigurator::makeThresholdRule (
        const std::string& rule_name,
        std::vector<std::string> topic_specification,
        const std::string& element_name,
        std::tuple <std::string, std::vector <std::string>, std::string, std::string> low_critical,
        std::tuple <std::string, std::vector <std::string>, std::string, std::string> low_warning,
        std::tuple <std::string, std::vector <std::string>, std::string, std::string> high_warning,
        std::tuple <std::string, std::vector <std::string>, std::string, std::string> high_critical,
        const char *lua_function)
{
    assert (topic_specification.size () >= 1);

    // target
    std::string target;
    if (topic_specification.size () == 1) {
        target = jsonify ("target", topic_specification [0]);
    }
    else {
        target = jsonify ("target", topic_specification);
    }

    // values
    std::string values =
        "[ { " + jsonify ("low_critical", std::get<0>(low_critical)) + " },\n"
        "  { " + jsonify ("low_warning", std::get<0>(low_warning)) + " },\n"
        "  { " + jsonify ("high_warning", std::get<0>(high_warning)) + " },\n"
        "  { " + jsonify ("high_critical", std::get<0>(high_critical)) + " } ]";

    // results
    std::string results =
        "[ { \"low_critical\"  : { " + jsonify ("action", std::get<1>(low_critical)) + ", " + jsonify ("severity", std::get<2>(low_critical))
        + ", " + jsonify ("description", std::get<3>(low_critical))  + " }},\n"
        "  { \"low_warning\"   : { " + jsonify ("action", std::get<1>(low_warning)) + ", " + jsonify ("severity", std::get<2>(low_warning))
        + ", " + jsonify ("description", std::get<3>(low_warning)) + " }},\n"
        "  { \"high_warning\"  : { " + jsonify ("action", std::get<1>(high_warning)) + ", " + jsonify ("severity", std::get<2>(high_warning))
        + ", " + jsonify ("description", std::get<3>(high_warning)) + " }},\n"
        "  { \"high_critical\" : { " + jsonify ("action", std::get<1>(high_critical)) + ", " + jsonify ("severity", std::get<2>(high_critical))
        + ", " + jsonify ("description", std::get<3>(high_critical))+ " }} ]";

    // evaluation
    std::string evaluation;
    if (lua_function) {
        evaluation = ",\n" + jsonify ("evaluation", lua_function);
    }


    std::string result =
        "{\n"
        "\"threshold\" : {\n"
        + jsonify ("rule_name", rule_name) + ",\n"
        + target + ",\n"
        + jsonify ("element", element_name) + ",\n"
        "\"values\" : " + values +",\n"
        "\"results\" : " + results
        + evaluation + "}\n"
        "}";

    return result;    
}

