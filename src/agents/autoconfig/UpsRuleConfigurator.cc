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
 \file   UpsRuleConfigurator.cc
 \brief  todo 
 \author Tomas Halman <TomasHalman@Eaton.com>
*/

#include "log.h"
#include "asset_types.h"

#include "UpsRuleConfigurator.h"

#define LUA_RULE(BIT) \
        " function has_bit(x,bit)"\
        "     local mask = 2 ^ (bit - 1)"\
        "     x = x % (2*mask)"\
        "     if x >= mask then return true else return false end"\
        " end"\
        " function main(status)"\
        "     if has_bit(status,"\
        #BIT\
        ") then return HIGH_CRITICAL end"\
        "     return OK"\
        " end"

bool UpsRuleConfigurator::v_configure (const std::string& name, const AutoConfigurationInfo& info, mlm_client_t *client)
{
    log_debug ("UpsRuleConfigurator::v_configure (name = '%s', info.type = '%" PRIi32"', info.subtype = '%" PRIi32"')",
        name.c_str(), info.type, info.subtype);    
    switch (info.operation) {
        case persist::asset_operation::INSERT:
        {
            // bits OB - 5 
            bool result = true;
            result &= sendNewRule (makeSingleRule (
                name + "_OnBattery", // rule_name
                std::vector<std::string>{"status.ups@" + name}, // target
                name, // element_name
                std::vector <std::pair <std::string, std::string>>{}, // values
                std::vector <std::tuple <std::string, std::vector <std::string>, std::string, std::string>> {
                    std::make_tuple ("high_critical", std::vector <std::string> {"EMAIL"}, "high", "UPS is running on battery!")
                }, // results
                LUA_RULE(5) // lua
                ), client);

            // LB - 7
            result &= sendNewRule (makeSingleRule (
                name + "_LowBattery", // rule_name
                std::vector<std::string>{"status.ups@" + name}, // target
                name, // element_name
                std::vector <std::pair <std::string, std::string>>{}, // values
                std::vector <std::tuple <std::string, std::vector <std::string>, std::string, std::string>> {
                    std::make_tuple ("high_critical", std::vector <std::string> {"EMAIL"}, "high", "Battery depleted!")
                }, // results
                LUA_RULE(7) // lua
                ), client);

            // BYPASS - 9
            result &= sendNewRule (makeSingleRule (
                name + "_OnBypass", // rule_name
                std::vector<std::string>{"status.ups@" + name}, // target
                name, // element_name
                std::vector <std::pair <std::string, std::string>>{}, // values
                std::vector <std::tuple <std::string, std::vector <std::string>, std::string, std::string>> {
                    std::make_tuple ("high_critical", std::vector <std::string> {"EMAIL"}, "high", "UPS is running on bypass!")
                }, // results
                LUA_RULE(9) // lua
                ), client);            
             
            return result;
            break;
        }
        case persist::asset_operation::UPDATE:
        case persist::asset_operation::DELETE:
        case persist::asset_operation::RETIRE:
        {
            break;
        }
        default:
        {
            log_warning ("todo");
            break;
        }
    }
    return true;
}

bool UpsRuleConfigurator::isApplicable (const AutoConfigurationInfo& info)
{
    if (info.type == persist::asset_type::DEVICE &&
        info.subtype == persist::asset_subtype::UPS) {
        return true;
    }
    return false;
}
