#include "alert.h"

#include <ctime>
#include <iostream>

alert_state_t Alert::state() const
{
    return _state;
}

void Alert::state(alert_state_t newState)
{
    if( _state != newState ) {
        std::cout << "changing state to " << newState << "\n";
        _state = newState;
        _published = 0;
        _persistenceInformed = false;
        _since = std::time(NULL);
    }
}

time_t Alert::since()
{
    if( _state == ALERT_STATE_ONGOING_ALERT ) return _since;
    return (time_t)0;
}

std::string Alert::name() { return _name; }
void Alert::name(const char *name) { _name = name; }
void Alert::name(const std::string &name) { _name = name; }

bool Alert::persistenceInformed() { return _persistenceInformed; }
void Alert::persistenceInformed(bool informed) { _persistenceInformed = informed; }

std::string Alert::devices() { return _devices; }
std::string Alert::description()
{
    //FIXME: do this
    return "";
}

alert_severity_t Alert::severity() { return _severity; }
void Alert::severity(alert_severity_t severity) { _severity = severity; }

bool Alert::timeToPublish() const
{
    if( _severity == ALERT_SEVERITY_UNKNOWN ) return false;
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
        if( device.length() ) {
            if( _devices.length() ) _devices += ",";
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
