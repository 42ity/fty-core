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

void Alert::addTopic( const std::string topic )
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
