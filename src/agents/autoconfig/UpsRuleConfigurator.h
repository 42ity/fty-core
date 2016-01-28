/*
Copyright (C) 2014 Eaton
 
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
 \file   UpsEpduRuleConfigurator.h
 \brief  Configuration of rules for ups and epdu devices
 \author Tomas Halman <TomasHalman@Eaton.com>
*/

#ifndef SRC_AGENTS_AUTOCONFIG_UPSRULECONFIGURATOR_H__
#define SRC_AGENTS_AUTOCONFIG_UPSRULECONFIGURATOR_H__

#include <string>

#include "RuleConfigurator.h"

class UpsRuleConfigurator : public RuleConfigurator {
 public:
    bool v_configure (const std::string& name, const AutoConfigurationInfo& info, mlm_client_t *client);
    bool isApplicable (const AutoConfigurationInfo& info);

    virtual ~UpsRuleConfigurator() {};
};

#endif // SRC_AGENTS_AUTOCONFIG_UPSRULECONFIGURATOR_H__

