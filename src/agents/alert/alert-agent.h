/*
Copyright (C) 2014 Eaton
 
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*! \file alert-agent.h
    \brief 
    \author Tomas Halman <tomashalman@eaton.com>
*/
 
#ifndef SRC_AGENTS_ALARMS_AGENT_H__
#define SRC_AGENTS_ALARMS_AGENT_H__

#include "bios_agent++.h"
#include "alert-model.h"
#include "alert-measurement.h"

class AlertAgent : public BIOSAgent {
 public:
    explicit AlertAgent( const char *agentName ) :BIOSAgent( agentName ) {  };
    explicit AlertAgent( const std::string &agentName ) :BIOSAgent( agentName ) { };
    void onSend( ymsg_t **message );
    void onReply( ymsg_t **message );
    void onStart();
    void onPoll();
 private:
    AlertModel _model;
};

#endif // SRC_AGENTS_ALARMS_AGENT_H__

