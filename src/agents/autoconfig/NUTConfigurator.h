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
 \file   NutConfigurator.h
 \brief  Class for nut configuration
 \author Tomas Halman <TomasHalman@Eaton.com>
*/
 
#ifndef SRC_AGENTS_AUTOCONFIG_NUTCONFIGURATOR_H__
#define SRC_AGENTS_AUTOCONFIG_NUTCONFIGURATOR_H__

#include <vector>
#include <string>

#include "Configurator.h"

class NUTConfigurator : public Configurator {
 public:
    bool v_configure (const std::string& name, const AutoConfigurationInfo& info, mlm_client_t *client);
    bool isApplicable (const AutoConfigurationInfo& info);

    // helper methods
    std::vector<std::string>::const_iterator selectBest (const std::vector<std::string>& configs);
    void updateNUTConfig ();
    virtual ~NUTConfigurator() {};

 private:
    std::vector<std::string>::const_iterator stringMatch (const std::vector<std::string>& texts, const char *pattern);
    bool match (const std::vector<std::string>& texts, const char *pattern);
    bool isEpdu (const std::vector<std::string>& texts);
    bool isUps (const std::vector<std::string>& texts);
    bool canSnmp (const std::vector<std::string>& texts);
    bool canXml (const std::vector<std::string>& texts);
    std::vector<std::string>::const_iterator getBestSnmpMib (const std::vector<std::string>& configs);
    void systemctl (const std::string& operation, const std::string& service);
};

#endif // SRC_AGENTS_AUTOCONFIG_NUTCONFIGURATOR_H__
