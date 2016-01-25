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

std::string UpsEpduRuleConfigurator::makeRule (const std::string& alert, const std::string& bit, const std::string& device, const std::string& description) const {
    return
        "{\n"
        "\"single\" : {\n"
        "    \"rule_name\"     :   \"" + alert + "-" + device + "\",\n"
        "    \"target\"        :   [\"status.ups@" + device + "\"],\n"
        "    \"values\"        :   [],\n"
        "    \"element\"       :   \"" + device + "\",\n"
        "    \"results\"       :   [ {\"high_critical\"  : { \"action\" : [ \"EMAIL\" ], \"description\" : \""+description+"\" }} ],\n"
        "    \"evaluation\"    : \""
        " function has_bit(x,bit)"
        "     local mask = 2 ^ (bit - 1)"
        "     x = x % (2*mask)"
        "     if x >= mask then return true else return false end"
        " end"
        " function main(status)"
        "     if has_bit(status,"+bit+") then return HIGH_CRITICAL end"
        "     return OK"
        " end"
        "\"\n"
        "  }\n"
        "}";
};

bool UpsEpduRuleConfigurator::v_configure (const std::string& name, const AutoConfigurationInfo& info, mlm_client_t *client)
{
    switch (info.operation) {
        case persist::asset_operation::INSERT:
        {
            // bits OB - 5 LB - 7 BYPASS - 9
            bool result = true;
            result &= sendNewRule (makeRule ("onbattery",  "5", name, "UPS is running on battery!"), client);
            result &= sendNewRule (makeRule ("lowbattery", "7", name, "Battery depleted!"), client);
            result &= sendNewRule (makeRule ("onbypass",   "9", name, "UPS is running on bypass!"), client);
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

