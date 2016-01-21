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
 \file   ConfiguratorFactory.h
 \brief  ConfiguratorFactory header file
 \author Tomas Halman <TomasHalman@Eaton.com>
*/

#ifndef SRC_AGENTS_AUTOCONFIG_CONFIGURATORFACTORY_H__
#define SRC_AGENTS_AUTOCONFIG_CONFIGURATORFACTORY_H__

#include <string>
#include <vector>

#include "Configurator.h"

class ConfiguratorFactory
{
 public:
    static std::vector <Configurator*> getConfigurator (uint32_t type, uint32_t subtype); // TODO: std::reference_wrapper

    ConfiguratorFactory (const ConfiguratorFactory&) = delete;
    void operator= (const ConfiguratorFactory&)      = delete;

 private:
    ConfiguratorFactory () {};
};


#endif // SRC_AGENTS_AUTOCONFIG_CONFIGURATORFACTORY_H__

