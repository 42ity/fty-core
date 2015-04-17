#include "alert-model.h"

#include <ctime>
#include <iostream>
#include "upsstatus.h"
#include "alert.h"

/* --------------------------- evaluating functions --------------------------- */

alert_state_t evaluate_on_battery_alert(
    const std::map<std::string, Measurement> &measurements,
    const std::map<std::string,std::string> &params
)
{
    // measurement suppose to be only one status.ups
    // params are not used
    params.size(); // silence compiler
    for( auto &it : measurements ) {
        if( it.second.source() == "status.ups" ) {
            if( it.second.value() & STATUS_OB ) { return ALERT_STATE_ONGOING_ALERT; }
            else { return ALERT_STATE_NO_ALERT; }
        }
    }
    return ALERT_STATE_UNKNOWN;
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
                // this is ups let's create simple alert configuration on battery
                Alert A;
                A.name("upsonbattery");
                A.severity( ALERT_SEVERITY_P5 ); //FIXME remove this and get it from DB
                A.addTopic( measurement.topic() );
                A.setEvaluateFunction( evaluate_on_battery_alert );
                _alerts.push_back(A);
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

std::vector<Alert> &AlertModel::alerts() {
    return _alerts;
}
