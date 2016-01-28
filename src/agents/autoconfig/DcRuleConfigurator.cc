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

#include "DcRuleConfigurator.h"

bool DcRuleConfigurator::v_configure (const std::string& name, const AutoConfigurationInfo& info, mlm_client_t *client)
{
    log_debug ("DcRuleConfigurator::v_configure (name = '%s', info.type = '%" PRIi32"', info.subtype = '%" PRIi32"')",
            name.c_str(), info.type, info.subtype);
    switch (info.operation) {
        case persist::asset_operation::INSERT:
        {
            bool result = true;
            result &= sendNewRule (makeSimpleThresholdRule (
                "DC_Temperature_Threshold",
                "average.temperature@"+name,
                name,
                std::make_tuple ("14", std::vector <std::string>{"EMAIL", "SMS"}, "high", "Temperature critically low in datacenter " + name), // low_critical
                std::make_tuple ("17", std::vector <std::string>{"EMAIL"}, "low", "Temperature low in datacenter "  + name), // low_warning
                std::make_tuple ("27", std::vector <std::string>{"EMAIL"}, "low", "Temperature high in datacenter " + name), // high_warning
                std::make_tuple ("30", std::vector <std::string>{"EMAIL", "SMS"}, "high", "Temperature critically high in datacenter " + name) // high_critical
                ), client);

            result &= sendNewRule (makeSimpleThresholdRule (
                "DC_Humidity_Threshold",
                "average.humidity@"+name,
                name,
                std::make_tuple ("30", std::vector <std::string>{"EMAIL", "SMS"}, "high", "Humidity critically low in datacenter " + name), // low_critical
                std::make_tuple ("40", std::vector <std::string>{"EMAIL"}, "low", "Humidity low in datacenter "  + name), // low_warning
                std::make_tuple ("60", std::vector <std::string>{"EMAIL"}, "low", "Humidity high in datacenter " + name), // high_warning
                std::make_tuple ("70", std::vector <std::string>{"EMAIL", "SMS"}, "high", "Humidity critically high in datacenter " + name) // high_critical
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

bool DcRuleConfigurator::isApplicable (const AutoConfigurationInfo& info)
{
    if (info.type == persist::asset_type::DATACENTER) {
        return true;
    }
    return false;
}
