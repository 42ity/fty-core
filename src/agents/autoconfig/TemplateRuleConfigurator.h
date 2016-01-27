/*
Copyright (C) 2014-2016 Eaton
 
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
 \file   TemplateRuleConfigurator.h
 \brief  Generic Configuration of rules 
 \author Gerald Guillaume <GeraldGuillaume@Eaton.com>
*/

#ifndef SRC_AGENTS_AUTOCONFIG_TEMPLATERULECONFIGURATOR_H
#define	SRC_AGENTS_AUTOCONFIG_TEMPLATERULECONFIGURATOR_H

#include <string>
#include <fstream>

#include "RuleConfigurator.h"


class TemplateRuleConfigurator : public RuleConfigurator {
 public:
    bool v_configure (const std::string& name, const AutoConfigurationInfo& info, mlm_client_t *client);
    bool isApplicable (const AutoConfigurationInfo& info);

    virtual ~TemplateRuleConfigurator() {};
private:
    bool checkTemplate(uint32_t type, uint32_t subtype);
    std::vector <std::string> loadTemplates(uint32_t type, uint32_t subtype);
    std::string convertTypeSubType2Name(uint32_t type, uint32_t subtype);
    std::string replaceTokens( const std::string &text, const std::string &pattern, const std::string &replacement) const;

    

};

#endif	/* SRC_AGENTS_AUTOCONFIG_TEMPLATERULECONFIGURATOR_H */




