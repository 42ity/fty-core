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

/*! \file alert-model.h
    \brief Class representing overall datacenter alerts
    \author Tomas Halman <tomashalman@eaton.com>
*/
 
#ifndef SRC_AGENTS_ALERT_ALERT_MODEL_H__
#define SRC_AGENTS_ALERT_ALERT_MODEL_H__

#include <string>
#include <map>
#include <vector>
#include "alert-measurement.h"
#include "alert.h"

class AlertModel {
 public:
    bool isMeasurementInteresting( const Measurement &measurement );
    void newMeasurement( const ymsg_t *message );
    void newMeasurement( const Measurement &Measurement );
    void addAlert( const Alert alert );
    void print();
    Alert *alertByRule(std::string ruleName);
    //std::vector<Alert>::iterator & alertByRule(std::string ruleName);
    std::vector<Alert> &alerts();
 protected:
    std::map< std::string, Measurement > _last_measurements;
    std::vector<Alert> _alerts;
};

#endif // SRC_AGENTS_ALERT_ALERT_MODEL_H__
