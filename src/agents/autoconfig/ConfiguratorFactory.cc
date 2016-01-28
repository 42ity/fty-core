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
 \file   ConfiguratorFactory.cc
 \brief  ConfiguratorFactory implementation
 \author Tomas Halman <TomasHalman@Eaton.com>
*/

#include <unistd.h>

#include "log.h"
#include "asset_types.h"

#include "NUTConfigurator.h"
#include "UpsRuleConfigurator.h"
#include "UptimeConfigurator.h"
#include "DCTHConfigurator.h"
#include "DcRuleConfigurator.h"
#include "DcRackRuleConfigurator.h"
#include "UpsEpduRuleConfigurator.h"

#include "ConfiguratorFactory.h"

std::vector <Configurator*> ConfiguratorFactory::getConfigurator (const AutoConfigurationInfo& info)
{
    static NUTConfigurator iNUTConfigurator;
    static UpsRuleConfigurator iUpsRuleConfigurator;
    static UptimeConfigurator iUptimeConfigurator;
    static DCTHConfigurator iDCTHConfigurator;
    static DcRuleConfigurator iDcRuleConfigurator;
    static DcRackRuleConfigurator iDcRackRuleConfigurator;
    static UpsEpduRuleConfigurator iUpsEpduRuleConfigurator;

    static std::vector <Configurator *> configurators {
        &iNUTConfigurator,
        &iUpsRuleConfigurator,
        &iUptimeConfigurator,
        &iDCTHConfigurator,
        &iDcRuleConfigurator,
        &iDcRackRuleConfigurator,
        &iUpsEpduRuleConfigurator
    };

    std::vector <Configurator *> retval;

    for (Configurator* item : configurators) {
        if (item->isApplicable (info)) {
            retval.push_back (item);
        }
    }

    return retval;
}

