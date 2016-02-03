/* 
Copyright (C) 2014 - 2016 Eaton
 
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
 \file   TemplateRuleConfigurator.cc
 \brief  use template file to generate a rule
 \author Gerald Guillaume <GeraldGuillaume@Eaton.com>
*/

#include "log.h"
#include <cxxtools/directory.h>
#include "TemplateRuleConfigurator.h"
#include "agent-autoconfig.h"
#include "asset_types.h"



bool TemplateRuleConfigurator::v_configure (const std::string& name, const AutoConfigurationInfo& info, mlm_client_t *client){
    log_debug ("TemplateRuleConfigurator::v_configure (name = '%s', info.type = '%" PRIi32"', info.subtype = '%" PRIi32"')",
            name.c_str(), info.type, info.subtype);
    switch (info.operation) {
        case persist::asset_operation::INSERT:
        {
            bool result = true;
            std::vector <std::string> templates = loadTemplates(info.type, info.subtype);
            for ( auto &templat : templates) {
                std::string rule=replaceTokens(templat,"__name__",name);
                log_debug("sending rule :\n %s", rule.c_str());
                result &= sendNewRule(rule,client);
            }

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

bool TemplateRuleConfigurator::isApplicable (const AutoConfigurationInfo& info){
    return checkTemplate(info.type, info.subtype);
}

std::vector <std::string> TemplateRuleConfigurator::loadTemplates(uint32_t type, uint32_t subtype){
    std::vector <std::string> templates;
    if (!cxxtools::Directory::exists (Autoconfig::StateFilePath)){
        log_info("TemplateRuleConfigurator '%s' dir does not exist",Autoconfig::StateFilePath);
        return templates;
    }
    std::string type_name = convertTypeSubType2Name(type,subtype);
    cxxtools::Directory d(Autoconfig::StateFilePath);
    for ( const auto &fn : d) {
        if ( fn.find(type_name.c_str())!= std::string::npos){
            log_debug("match %s", fn.c_str());
            // read the template rule from the file
            std::ifstream f(d.path() + "/" + fn);
            std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            templates.push_back(str);
        }
    }
    return templates;
    
}

bool TemplateRuleConfigurator::checkTemplate(uint32_t type, uint32_t subtype){
    if (!cxxtools::Directory::exists (Autoconfig::StateFilePath)){
        log_info("TemplateRuleConfigurator '%s' dir does not exist",Autoconfig::StateFilePath);
        return false;
    }
    std::string type_name = convertTypeSubType2Name(type,subtype);
    cxxtools::Directory d(Autoconfig::StateFilePath);
    for ( const auto &fn : d) {
        if ( fn.find(type_name.c_str())!= std::string::npos){
            return true;
        }
    }
    return false;
    
}

std::string TemplateRuleConfigurator::convertTypeSubType2Name(uint32_t type, uint32_t subtype){
    std::string name;
    if(type == persist::asset_type::DEVICE){
        name= "__" + persist::typeid_to_type(type) + "_" + persist::subtypeid_to_subtype(subtype) + "__";
    }else{
        name= "__" + persist::typeid_to_type(type) + "__";
    }
    log_debug("convertTypeSubType2Name(info.type = '%" PRIi32"', info.subtype = '%" PRIi32"' = '%s')",
            type, subtype,name.c_str());
    return name;
    
}

std::string TemplateRuleConfigurator::replaceTokens( const std::string &text, const std::string &pattern, const std::string &replacement) const{
    std::string result = text;
    size_t pos = 0;
    while( ( pos = result.find(pattern, pos) ) != std::string::npos){
        result.replace(pos, pattern.length(), replacement);
        pos += replacement.length();
    }
    return result;
}
