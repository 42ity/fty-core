/*
Copyright (C) 2015 Eaton
 
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

/*! \file calc_power.h
    \brief Functions for calculating a total rack and DC power from 
     database values.
    \author Alena Chernikava <alenachernikava@eaton.com>
*/

#ifndef SRC_PERSIST_CALC_POWER_H_
#define SRC_PERSIST_CALC_POWER_H_

#include <set>
#include <tuple>

#include "common_msg.h"
#include "dbtypes.h"

#include "assettopology.h"

#define DEVICE_TYPE_EPDU 3
#define DEVICE_TYPE_PDU 4
#define DEVICE_TYPE_UPS 1
#define DEVICE_TYPE_SERVER 5


typedef 
        std::tuple < std::set < device_info_t >, 
                     std::set < device_info_t >, 
                     std::set < device_info_t>  
                    >
        power_sources_t;



power_sources_t doA ( const char* url,
                      std::pair < std::set < device_info_t >, 
                      std::set < powerlink_info_t > > power_topology, 
          device_info_t start_device );

common_msg_t* calc_total_rack_power (const char *url, a_elmnt_id_t rack_element_id);
#endif //SRC_PERSIST_CALC_POWER_H_
