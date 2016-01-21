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
 \file   UptimeConfigurator.h
 \brief  Configuration of uptime
 \author Tomas Halman <TomasHalman@Eaton.com>
*/

#ifndef SRC_AGENTS_AUTOCONFIG_UPTIMECONFIGURATOR_H__
#define SRC_AGENTS_AUTOCONFIG_UPTIMECONFIGURATOR_H__

#include <string>

#include "Configurator.h"

class UptimeConfigurator : public Configurator {
 public:
    bool v_configure (const std::string& name, const AutoConfigurationInfo& info, mlm_client_t *client);

    // helper methods
    bool    obtainData (std::map <std::string, std::vector <std::string>>& dc_upses);
    bool    sendMessage (std::map <std::string, std::vector <std::string>>& dc_upses, mlm_client_t *client);

    virtual ~UptimeConfigurator () {};
};

#endif // SRC_AGENTS_AUTOCONFIG_UPTIMECONFIGURATOR_H__
