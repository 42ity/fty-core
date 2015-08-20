/*
 *
 * Copyright (C) 2015 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*!
 * \file alert-entity.cc
 * \author Tomas Halman
 * \author Alena Chernikava
 * \brief Not yet documented file
 */
#include "alert-entity.h"

#include <ctime>

#include "log.h"

alert_state_t Alert::state() const
{
    return _state;
}

void Alert::state(alert_state_t newState)
{
    if( _state != newState ) {
        if( newState != ALERT_STATE_UNKNOWN ) {
            log_info("alert: %s (%s) %s",
                     ruleName().c_str(),
                     description().c_str(),
                     ( newState == ALERT_STATE_ONGOING_ALERT ? "active!" : "OK (not active)." ) );
        }
        _state = newState;
        _published = 0;
        _persistenceInformed = false;
        _since = std::time(NULL);
    }
}

time_t Alert::since() const
{
    if( _state == ALERT_STATE_UNKNOWN ) return time_t(0);
    return _since;
}

bool Alert::timeToPublish() const
{
    if( _priority == ALERT_PRIORITY_UNKNOWN ) return false;
    switch( _state ) {
    case ALERT_STATE_UNKNOWN:
        return false;
    case ALERT_STATE_NO_ALERT:
        return time(NULL) - _published >= NO_ALERT_PUBLISH_INTERVAL;
    case ALERT_STATE_ONGOING_ALERT:
        return time(NULL) - _published >= ONGOING_ALERT_PUBLISH_INTERVAL;
    }
    return false;
}

void Alert::published() {
    _published = std::time(NULL);
}

void Alert::setEvaluateFunction( alert_evaluate_function_t func )
{
    _evaluate = func;
    _evaluateParameters.clear();
}

void Alert::setEvaluateFunction( alert_evaluate_function_t func, std::map<std::string,std::string> params )
{
    _evaluate = func;
    _evaluateParameters = params;
}

void Alert::addEvaluateFunctionParameter( const std::string &param, const std::string &value )
{
    _evaluateParameters[param] = value;
}

void Alert::addTopic( const std::string &topic )
{
    _measurementTopics.push_back( topic );
    size_t pos = topic.find("@");
    if( pos != std::string::npos ) {
        std::string device = topic.substr( pos + 1 );
        if( ! device.empty() ) {
            if( ! _devices.empty() ) _devices += ",";
            _devices += device;
        }
    }
}

alert_state_t Alert::evaluate( const std::map<std::string, Measurement> &measurementAvailable )
{
    // do we have evaluate function?
    if( _evaluate == NULL ) {
        state(ALERT_STATE_UNKNOWN);
        return _state;
    }
    // do we have all necessary measurements?
    std::map <std::string, Measurement> localMeasurement;
    for( auto &it : _measurementTopics ) {
        auto measurement = measurementAvailable.find(it);
        if( measurement == measurementAvailable.end() ) {
            state(ALERT_STATE_UNKNOWN);
            return _state;
        }
        localMeasurement[it] = measurement->second;
    }
    // evaluate
    state( _evaluate( localMeasurement, _evaluateParameters ) );    
    return _state;
}
