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
 \file   DCTHConfigurator.cc
 \brief  Configuration of composite-metrics
 \author Michal Vyskocil <michalvyskocil@eaton.com>
*/

#include "log.h"
#include "asset_types.h"
#include "preproc.h"

#include "bits.h"
#include "DCTHConfigurator.h"

static bool
    s_is_rc (const AutoConfigurationInfo &info)
{
    // Form ID from hostname and agent name
    char hostname[HOST_NAME_MAX];
    ::gethostname(hostname, HOST_NAME_MAX);
    return (info.attributes.count("hostname.1") == 1 && info.attributes.at("hostname.1") == hostname);
}


bool DCTHConfigurator::v_configure (UNUSED_PARAM const std::string& name, const AutoConfigurationInfo& info, UNUSED_PARAM mlm_client_t *client)
{
    log_debug ("DCTHConfigurator::v_configure (name = '%s', info.type = '%" PRIi32"', info.subtype = '%" PRIi32"')",
        name.c_str(), info.type, info.subtype);
    switch (info.operation) {
        case persist::asset_operation::INSERT:
        case persist::asset_operation::UPDATE:
        case persist::asset_operation::DELETE:
            {
                bits::systemctl ("restart", "dc_th");
            }
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

bool DCTHConfigurator::isApplicable (const AutoConfigurationInfo& info)
{
    if (info.type == persist::asset_type::DATACENTER || s_is_rc (info)) {
        return true;
    }
    return false;
}
