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
 \file   UpsEpduRuleConfigurator.cc
 \brief  todo 
 \author Tomas Halman <TomasHalman@Eaton.com>
*/

#include "log.h"
#include "asset_types.h"

#include "DcRackRuleConfigurator.h"

#define DCRACK_TotalPower_3Phase_Threshold_LUA \
    "function main(p1,p2,p3)"\
    "   local suma = p1 + p2 + p3;"\
    "   if (suma > high_critical) then return HIGH_CRITICAL end;"\
    "   if (suma > high_warning) then return HIGH_WARNING end;"\
    "   return OK;"\
    "end"

#define DCRACK_PhaseImbalance_Threshold_LUA \
    "function main(f1,f2,f3)"\
    "   local avg = (f1 + f2 + f3) / 3;"\
    "   local deviation = math.max (math.abs (f1 - avg), math.abs (f2 - avg), math.abs (f3 - avg));"\
    "   local percentage = deviation / avg * 100;"\
    "   if (percentage > high_critical) then return HIGH_CRITICAL end;"\
    "   if (percentage > high_warning) then return HIGH_WARNING end;"\
    "   return OK;"\
    "end"

bool DcRackRuleConfigurator::v_configure (const std::string& name, const AutoConfigurationInfo& info, mlm_client_t *client)
{
    log_debug ("DcRackRuleConfigurator::v_configure (name = '%s', info.type = '%" PRIi32"', info.subtype = '%" PRIi32"')",
            name.c_str(), info.type, info.subtype);
    switch (info.operation) {
        case persist::asset_operation::INSERT:
        {
            bool result = true;
            result &= sendNewRule (makeSimpleThresholdRule (
                "DC_RACK_TotalPower_1Phase_Threshold",
                "realpower.input.L1@"+name,
                name,
                std::make_tuple ("0", std::vector <std::string>{"EMAIL"}, "high", "Total power (1 phase) in datacenter " + name + " is critically low"), // low_critical
                std::make_tuple ("0", std::vector <std::string>{"EMAIL"}, "low", "Total power (1 phase) in datacenter " + name + " is low"), // low_warning
                std::make_tuple ("0", std::vector <std::string>{"EMAIL"}, "low", "Total power (1 phase) in datacenter " + name + " is high"), // high_warning
                std::make_tuple ("0", std::vector <std::string>{"EMAIL"}, "high", "Total power (1 phase) in datacenter " + name + " is critically high") // high_critical
                ), client);

            result &= sendNewRule (makeThresholdRule (
                "DC_RACK_TotalPower_3Phase_Threshold",
                std::vector<std::string>{"realpower.input.L1@" + name, "realpower.input.L2@" + name, "realpower.input.L3@" + name},
                name,
                std::vector <std::pair <std::string, std::string>>{ {"high_warning","0"}, {"high_critical","0"}},
                std::vector <std::tuple <std::string, std::vector <std::string>, std::string, std::string>> {
                    std::make_tuple ("high_warning", std::vector <std::string> {"EMAIL"}, "high", "Total power (3 phase) in datacenter " + name + " is high"),
                    std::make_tuple ("high_critical", std::vector <std::string> {"EMAIL"}, "high", "Total power (3 phase) in datacenter " + name + " is critically high"),
                },
                DCRACK_TotalPower_3Phase_Threshold_LUA
                ), client);

            result &= sendNewRule (makeThresholdRule (
                "DC_RACK_PhaseImbalance_Threshold",
                std::vector<std::string>{"realpower.output.L1@" + name, "realpower.output.L2@" + name, "realpower.output.L3@" + name},
                name,
                std::vector <std::pair <std::string, std::string>>{ {"high_warning","10"}, {"high_critical","20"}},
                std::vector <std::tuple <std::string, std::vector <std::string>, std::string, std::string>> {
                    std::make_tuple ("high_warning", std::vector <std::string> {"EMAIL"}, "high", "Phase imbalance in datacenter " + name + " is high"),
                    std::make_tuple ("high_critical", std::vector <std::string> {"EMAIL"}, "high", "Phase imbalance in datacenter " + name + " is critically high"),
                },
                DCRACK_PhaseImbalance_Threshold_LUA
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

bool DcRackRuleConfigurator::isApplicable (const AutoConfigurationInfo& info)
{
    if (info.type == persist::asset_type::DATACENTER ||
        info.type == persist::asset_type::RACK) {
        return true;
    }
    return false;
}

