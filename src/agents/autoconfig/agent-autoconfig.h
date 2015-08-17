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

/*! \file   agent-autoconfig.h
    \brief  autoconfiguration agent
    \author Tomas Halman <TomasHalman@Eaton.com>
*/
 
#ifndef SRC_AGENTS_AUTOCONFIG_AGENT_H__
#define SRC_AGENTS_AUTOCONFIG_AGENT_H__

#include "bios_agent++.h"
#include "configurator.h"

class Autoconfig : public BIOSAgent {
 public:
    explicit Autoconfig( const char *agentName ) :BIOSAgent( agentName ) {  };
    explicit Autoconfig( const std::string &agentName ) :BIOSAgent( agentName ) { };

    void onStart( );
    void onEnd( ) { saveState(); };
    void onReply( ymsg_t **message );
    void onSend( ymsg_t **message );
    void onPoll( );
 private:
    void saveState();
    void loadState();
    ConfigFactory _configurator;
    std::map<std::string,AutoConfigurationInfo> _configurableDevices;
};

#endif // SRC_AGENTS_AUTOCONFIG_AGENT_H__

