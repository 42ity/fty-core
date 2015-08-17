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

/*! \file   configurator.h
    \brief  Classes for autoconfiguration
    \author Tomas Halman <TomasHalman@Eaton.com>
*/
 
#ifndef SRC_AGENTS_AUTOCONFIG_CONFIGURATOR_H__
#define SRC_AGENTS_AUTOCONFIG_CONFIGURATOR_H__

#include <map>
#include <vector>
#include <string>

#include "ymsg.h"
#include "log.h"
#include "nutscan.h"

struct AutoConfigurationInfo
{
    std::string name;
    std::string type;
    bool configured;
    time_t date;
    std::map<std::string,std::string> attributes;
};

class Configurator {
 public:
    virtual ~Configurator() {};
    virtual bool configure( const char *name, const std::map<std::string,std::string> &extendedAttributes, int8_t eventType );
};

class NUTConfigurator : public Configurator {
 public:
    virtual ~NUTConfigurator() {};
    std::vector<std::string>::const_iterator selectBest( const std::vector<std::string> &configs);
    void updateNUTConfig();
    bool configure(  const char *name, const std::map<std::string,std::string> &extendedAttributes, int8_t eventType );
 private:
    std::vector<std::string>::const_iterator stringMatch( const std::vector<std::string> &texts, const char *pattern);
    bool match( const std::vector<std::string> &texts, const char *pattern);
    bool isEpdu( const std::vector<std::string> &texts);
    bool isUps( const std::vector<std::string> &texts);
    bool canSnmp( const std::vector<std::string> &texts);
    bool canXml( const std::vector<std::string> &texts);
    std::vector<std::string>::const_iterator getBestSnmpMib( const std::vector<std::string> &configs);
};

class ConfigFactory {
 public:
    bool configureAsset(AutoConfigurationInfo &info);
 private:
    Configurator * getConfigurator(std::string type);
};

#endif // SRC_AGENTS_AUTOCONFIG_CONFIGURATOR_H__
