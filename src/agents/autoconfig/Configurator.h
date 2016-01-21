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
 \file   Configurator.h
 \brief  Configurator class
 \author Tomas Halman <TomasHalman@Eaton.com>
*/

#ifndef SRC_AGENTS_AUTOCONFIG_CONFIGURATOR_H__
#define SRC_AGENTS_AUTOCONFIG_CONFIGURATOR_H__

#include <string>
#include <map>
#include <vector>

#include <malamute.h>

struct AutoConfigurationInfo
{
    uint32_t type = 0;
    uint32_t subtype = 0;
    int8_t operation;
    bool configured = false;
    time_t date = 0;
    std::map <std::string, std::string> attributes;
};

class Configurator
{
 public:
    bool configure (const std::string& name, const AutoConfigurationInfo& info)
    {
        return configure (name, info, NULL);
    }

    bool configure (const std::string& name, const AutoConfigurationInfo& info, mlm_client_t *client)
    {
        return v_configure (name, info, client);
    }

    virtual ~Configurator() {};

 protected:
    virtual bool v_configure (const std::string& name, const AutoConfigurationInfo& info, mlm_client_t *client) = 0;
};

#endif // SRC_AGENTS_AUTOCONFIG_CONFIGURATOR_H__
