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
 * \file alert-model.cc
 * \author Tomas Halman
 * \brief Not yet documented file
 */
#include "alert-model.h"

#include <ctime>
#include <iostream>
#include <tntdb/connect.h>

#include "upsstatus.h"
#include "assetcrud.h"
#include "dbpath.h"
#include "log.h"

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

bool  AlertModel::isMeasurementInteresting( const Measurement &measurement ) const
{
    return ( measurement.source() == "status.ups" );
}

void AlertModel::newMeasurement( const ymsg_t *message )
{
    newMeasurement( Measurement( message ) );
}

alert_priority_t AlertModel::devicePriority(const std::string &deviceName) {
    if( deviceName.empty() ) return ALERT_PRIORITY_UNKNOWN;
    try {
        auto connection = tntdb::connect(url);
        auto element = select_asset_element_by_name(connection, deviceName.c_str() );
        connection.close();
        if( element.status ) {
            log_debug("Device %s priority is %" PRIu8, deviceName.c_str(), element.item.priority );
            return (alert_priority_t)element.item.priority;
        }
    } catch (...) { }
    log_error("Failed to get priority of %s", deviceName.c_str() );
    return ALERT_PRIORITY_UNKNOWN;
}

void AlertModel::newMeasurement( const Measurement &measurement )
{
    if( isMeasurementInteresting( measurement ) ) {
        if( _last_measurements.find(measurement.topic()) == _last_measurements.end() ) {
            // this is something new
            if( measurement.source() == "status.ups" ) {
                // this is ups let's create simple alert configuration
                // get device priority
                
                alert_priority_t P = devicePriority( measurement.deviceName() );
                if( P == ALERT_PRIORITY_UNKNOWN ) return;
                Alert A;
                A.name("upsonbattery");
                A.addTopic( measurement.topic() );
                A.setEvaluateFunction( evaluate_on_battery_alert );
                A.description("UPS is running on battery!");
                A.priority(P);
                _alerts[A.ruleName()] = A;
                
                Alert B;
                B.name("upslowbattery");
                B.addTopic( measurement.topic() );
                B.setEvaluateFunction( evaluate_low_battery_alert );
                B.description("Low battery!");
                B.priority(P);
                _alerts[B.ruleName()] = B;

                Alert C;
                C.name("upsonbypass");
                C.addTopic( measurement.topic() );
                C.setEvaluateFunction( evaluate_on_bypass_alert );
                C.description("UPS is on bypass!");
                C.priority(P);
                _alerts[C.ruleName()] = C;
            }
        }
        _last_measurements[ measurement.topic() ] = measurement;
        for( auto &it: _alerts ) {
            it.second.evaluate( _last_measurements );
        }
    }
}

void AlertModel::print() const
{
    for( auto &it: _last_measurements ) {
        std::cout << "\n" << it.first << "\n";
        it.second.print();
    }
}

std::map<std::string, Alert>::iterator AlertModel::alertByRule(std::string ruleName)
{
    return _alerts.find(ruleName);
}

std::map<std::string, Alert>::iterator AlertModel::end()
{
    return _alerts.end();
}

std::map<std::string, Alert>::iterator AlertModel::begin()
{
    return _alerts.begin();
}

void AlertModel::setPriority( std::string device, alert_priority_t priority )
{
    // TODO: this simple solution (alert priority = device priority) suppose to be
    // replaced by taking alert priority from configuration
    for( auto &al: _alerts )
        if( al.second.devices() == device ) al.second.priority(priority);
}
