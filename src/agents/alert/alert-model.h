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

/*! \file   alert-model.h
    \brief  Class representing overall datacenter alerts
    \author Tomas Halman <TomasHalman@Eaton.com>
*/
 
#ifndef SRC_AGENTS_ALERT_ALERT_MODEL_H__
#define SRC_AGENTS_ALERT_ALERT_MODEL_H__

#include <string>
#include <map>
#include <vector>
#include "alert-measurement.h"
#include "alert-entity.h"

class AlertModel {
 public:
    bool isMeasurementInteresting( const Measurement &measurement ) const;
    void newMeasurement( const ymsg_t *message );
    void newMeasurement( const Measurement &Measurement );
    void addAlert( const Alert &alert );
    void print() const;

    std::map<std::string, Alert>::iterator alertByRule(std::string ruleName);
    std::map<std::string, Alert>::iterator begin();
    std::map<std::string, Alert>::iterator end();

    void setPriority( std::string device, alert_priority_t priority );
 protected:
    alert_priority_t devicePriority(const std::string &deviceName);
    std::map< std::string, Measurement > _last_measurements;
    std::map<std::string, Alert> _alerts;
};

#endif // SRC_AGENTS_ALERT_ALERT_MODEL_H__
