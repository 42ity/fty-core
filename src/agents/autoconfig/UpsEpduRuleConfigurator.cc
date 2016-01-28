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

#include "UpsEpduRuleConfigurator.h"

#define UPSEPDU_InputVoltage_3Phase_Threshold_LUA \
    "function main(v1,v2,v3)"\
    "   if (v1 > high_critical or v2 > high_critical or v3 > high_critical) then return HIGH_CRITICAL end;"\
    "   if (v1 > high_warning or v2 > high_warning or v3 > high_warning) then return HIGH_WARNING end;"\
    "   if (v1 < low_critical or v2 < low_critical or v3 < low_critical) then return LOW_CRITICAL end;"\
    "   if (v1 < low_warning or v2 < low_warning or v3 < low_warning) then return LOW_WARNING end;"\
    "   return OK;"\
    "end"

#define UPSEPDU_PhaseImbalance_Threshold_LUA \
    "function main(f1,f2,f3)"\
    "   local avg = (f1 + f2 + f3) / 3;"\
    "   local deviation = math.max (math.abs (f1 - avg), math.abs (f2 - avg), math.abs (f3 - avg));"\
    "   local percentage = deviation / avg * 100;"\
    "   if (percentage > high_critical) then return HIGH_CRITICAL end;"\
    "   if (percentage > high_warning) then return HIGH_WARNING end;"\
    "   return OK;"\
    "end"


bool UpsEpduRuleConfigurator::v_configure (const std::string& name, const AutoConfigurationInfo& info, mlm_client_t *client)
{
    log_debug ("UpsEpduRuleConfigurator::v_configure (name = '%s', info.type = '%" PRIi32"', info.subtype = '%" PRIi32"')",
        name.c_str(), info.type, info.subtype);
    switch (info.operation) {
        case persist::asset_operation::INSERT:
        {
            bool result = true;
       
            result &= sendNewRule (makeSimpleThresholdRule (
                "UPSEPDU_InputVoltage_1Phase_Threshold",
                "voltage.input.L1@"+name,
                name,
                std::make_tuple ("0", std::vector <std::string>{"EMAIL"}, "high", "Input voltage (1 phase) in device" + name + " is critically low"), // low_critical
                std::make_tuple ("0", std::vector <std::string>{"EMAIL"}, "low", "Input voltage (1 phase) in device " + name + " is low"), // low_warning
                std::make_tuple ("0", std::vector <std::string>{"EMAIL"}, "low", "Input voltage (1 phase) in device " + name + " is high"), // high_warning
                std::make_tuple ("0", std::vector <std::string>{"EMAIL"}, "high", "Input voltage (1 phase) in device " + name + " is critically high") // high_critical
                ), client);

            result &= sendNewRule (makeThresholdRule (
                "UPSEPDU_InputVoltage_3Phase_Threshold",
                std::vector<std::string>{"voltage.input.L1@" + name, "voltage.input.L2@" + name, "voltage.input.L3@" + name},
                name,
                std::vector <std::pair <std::string, std::string>>{
                    {"low_critical","210"},
                    {"low_warning","215"},
                    {"high_warning","235"},
                    {"high_critical","240"}
                    },
                std::vector <std::tuple <std::string, std::vector <std::string>, std::string, std::string>> {
                    std::make_tuple ("low_critical", std::vector <std::string> {"EMAIL"}, "high", "Input voltage (3 phase) in device " + name + " is critically low"),
                    std::make_tuple ("low_warning", std::vector <std::string> {"EMAIL"}, "high", "Input voltage (3 phase) in device " + name + " is low"),
                    std::make_tuple ("high_warning", std::vector <std::string> {"EMAIL"}, "high", "Input voltage (3 phase) in device " + name + " is high"),
                    std::make_tuple ("high_critical", std::vector <std::string> {"EMAIL"}, "high", "Input voltage (3 phase) in device " + name + " is critically high"),
                },
                UPSEPDU_InputVoltage_3Phase_Threshold_LUA
                ), client);

            result &= sendNewRule (makeThresholdRule (
                "UPSEPDU_PhaseImbalance_Threshold",
                std::vector<std::string>{"realpower.output.L1@" + name, "realpower.output.L2@" + name, "realpower.output.L3@" + name},
                name,
                std::vector <std::pair <std::string, std::string>>{ {"high_warning","10"}, {"high_critical","20"}},
                std::vector <std::tuple <std::string, std::vector <std::string>, std::string, std::string>> {
                    std::make_tuple ("high_warning", std::vector <std::string> {"EMAIL"}, "high", "Phase imbalance in device " + name + " is high"),
                    std::make_tuple ("high_critical", std::vector <std::string> {"EMAIL"}, "high", "Phase imbalance in device " + name + " is critically high"),
                },
                UPSEPDU_PhaseImbalance_Threshold_LUA
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

bool UpsEpduRuleConfigurator::isApplicable (const AutoConfigurationInfo& info)
{
    if (info.type == persist::asset_type::DEVICE &&
        (info.subtype == persist::asset_subtype::UPS || info.subtype == persist::asset_subtype::EPDU)) {
        return true;
    }
    return false;
}

