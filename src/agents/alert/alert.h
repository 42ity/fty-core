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

/*! \file alert.h
    \brief Class for alert evaluation
    \author Tomas Halman <tomashalman@eaton.com>
*/
 
#ifndef SRC_AGENTS_ALERT_ALERT_H__
#define SRC_AGENTS_ALERT_ALERT_H__

#include <string>
#include <map>
#include <vector>
#include "ymsg.h"
#include "agents.h"

#include "alert-measurement.h"

#define NO_ALERT_PUBLISH_INTERVAL 20
#define ONGOING_ALERT_PUBLISH_INTERVAL 5

typedef alert_state_t (*alert_evaluate_function_t)(
    const std::map<std::string, Measurement> &measurements,
    const std::map<std::string,std::string> &params
);

class Alert {
 public:
    alert_state_t state() const;
    void state(alert_state_t newState);

    std::string name() const { return _name; };
    void name(const char *text) { _name = text; };
    void name(const std::string &text) { _name = text; };
    
    std::string ruleName() const  { return _name + "@" + _devices; };
    
    bool persistenceInformed() const { return _persistenceInformed; };
    void persistenceInformed(bool informed) { _persistenceInformed = informed; };
    
    std::string devices() const { return _devices; }

    std::string description() const { return _description; }
    void description(const char* text) { _description = text; }
    void description(const std::string &text) { _description = text; }

    alert_priority_t priority() const { return _priority; }
    void priority(alert_priority_t priority) { _priority = priority; }

    time_t since() const;
    bool timeToPublish() const;
    void published();
    void setEvaluateFunction( alert_evaluate_function_t func );
    void setEvaluateFunction( alert_evaluate_function_t func, std::map<std::string,std::string> params );
    void addEvaluateFunctionParameter( const std::string &param, const std::string &value );
    void addTopic( const std::string topic );
    alert_state_t evaluate( const std::map< std::string, Measurement > &measurementAvailable );
 protected:
    alert_state_t _state = ALERT_STATE_UNKNOWN;
    alert_priority_t _priority = ALERT_PRIORITY_UNKNOWN;
    
    time_t _since = 0;
    time_t _published = 0;
    bool _persistenceInformed = false;
    std::string _name;
    std::string _devices;
    std::string _description;
    
    alert_evaluate_function_t _evaluate = NULL;
    std::map<std::string,std::string> _evaluateParameters;
    std::vector<std::string> _measurementTopics; // list of topics/inputs
};

#endif // SRC_AGENTS_ALERT_ALERT_H__
