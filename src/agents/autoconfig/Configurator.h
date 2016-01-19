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
 \brief  Configurator header
 \author Tomas Halman <TomasHalman@Eaton.com>
*/

#ifndef SRC_AGENTS_AUTOCONFIG_CONFIGURATOR_H__
#define SRC_AGENTS_AUTOCONFIG_CONFIGURATOR_H__

#include <string>
#include <map>
#include <vector>

#include "asset_types.h"

struct AutoConfigurationInfo
{
    uint32_t type = 0;
    uint32_t subtype = 0;
    persist::asset_operation operation;
    bool configured = false;
    time_t date = 0;
    std::map <std::string, std::string> attributes;
};

class Configurator
{
 public:
    //! TODO declare what is this method expected to do for subclass implementors
    virtual bool configure (const std::string& name, const AutoConfigurationInfo& info);
    //! TODO declare what is this method expected to do for subclass implementors
    virtual std::vector<std::string> createRules(std::string const &name);

    virtual ~Configurator() {};
};

#endif // SRC_AGENTS_AUTOCONFIG_CONFIGURATOR_H__
