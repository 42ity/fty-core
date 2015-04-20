#include "alert-model.h"

#include <ctime>
#include <iostream>
#include "upsstatus.h"
#include "alert.h"

/* --------------------------- evaluating functions --------------------------- */

alert_state_t evaluate_ups_status(
    const std::map<std::string, Measurement> &measurements,
    uint32_t mask
)
{
    // measurement suppose to be only one status.ups
    for( auto &it : measurements ) {
        if( it.second.source() == "status.ups" ) {
            if( it.second.value() & mask ) { return ALERT_STATE_ONGOING_ALERT; }
            else { return ALERT_STATE_NO_ALERT; }
        }
    }
    return ALERT_STATE_UNKNOWN;
}

alert_state_t evaluate_on_battery_alert(
    const std::map<std::string, Measurement> &measurements,
    const std::map<std::string,std::string> &params
)
{
    // params are not used
    params.size(); // silence compiler
    return evaluate_ups_status(measurements,STATUS_OB);
}

alert_state_t evaluate_low_battery_alert(
    const std::map<std::string, Measurement> &measurements,
    const std::map<std::string,std::string> &params
)
{
    // params are not used
    params.size(); // silence compiler
    return evaluate_ups_status(measurements,STATUS_LB);
}

alert_state_t evaluate_on_bypass_alert(
    const std::map<std::string, Measurement> &measurements,
    const std::map<std::string,std::string> &params
)
{
    // params are not used
    params.size(); // silence compiler
    return evaluate_ups_status(measurements,STATUS_BYPASS);
}

/* --------------------------- AlertModel --------------------------- */

bool  AlertModel::isMeasurementInteresting( const Measurement &measurement )
{
    return ( measurement.source() == "status.ups" );
}

void AlertModel::newMeasurement( const ymsg_t *message )
{
    newMeasurement( Measurement( message ) );
}

void AlertModel::newMeasurement( const Measurement &measurement )
{
    if( isMeasurementInteresting( measurement ) ) {
        if( _last_measurements.find(measurement.topic()) == _last_measurements.end() ) {
            // this is something new
            if( measurement.source() == "status.ups" ) {
                // this is ups let's create simple alert configuration
                Alert A;
                A.name("upsonbattery");
                A.priority( ALERT_PRIORITY_P5 ); //FIXME remove this and get it from DB
                A.addTopic( measurement.topic() );
                A.setEvaluateFunction( evaluate_on_battery_alert );
                _alerts.push_back(A);
                
                Alert B;
                B.name("upslowbattery");
                B.priority( ALERT_PRIORITY_P5 ); //FIXME remove this and get it from DB
                B.addTopic( measurement.topic() );
                B.setEvaluateFunction( evaluate_low_battery_alert );
                _alerts.push_back(B);

                Alert C;
                C.name("upsonbypass");
                C.priority( ALERT_PRIORITY_P5 ); //FIXME remove this and get it from DB
                C.addTopic( measurement.topic() );
                C.setEvaluateFunction( evaluate_on_bypass_alert );
                _alerts.push_back(C);
            }
        }
        _last_measurements[ measurement.topic() ] = measurement;
        // FIXME: do the following only on measurement change?
        for( auto &it: _alerts ) {
            it.evaluate( _last_measurements );
        }
    }
}

void AlertModel::print()
{
    for( auto &it: _last_measurements ) {
        std::cout << "\n" << it.first << "\n";
        it.second.print();
    }
}

Alert *AlertModel::alertByRule(std::string ruleName)
{
    for( auto &it: _alerts ) {
        if( it.ruleName() == ruleName ) {
            return &(it);
        }
    }
    return NULL;
}

std::vector<Alert> &AlertModel::alerts() {
    return _alerts;
}
