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

/*! \file dbhelpers.h
    \brief Helper function for direct interact with DB
    \author Alena Chernikava <alenachernikava@eaton.com>
*/

#ifndef SRC_PERSIST_DBHELPERS_H_
#define SRC_PERSIST_DBHELPERS_H_

#include "dbtypes.h"

// For now it the maximum length of all fields 
// called "name"

#define MAX_NAME_LENGTH 25
#define MAX_DESCRIPTION_LENGTH 255

/**
 * \brief This function looks for a device_discovered from a monitor part 
 * connected with the specified asset_element in the asset part.
 *
 * Throws exceptions: bios::MonitorCounterpartNotFound - in case apropriate 
 *                              monitor element was not found.
 *                    bios::InternalDBError - in case of any database errors.
 *                    bios::ElementIsNotDevice - if specified element was 
 *                              not a device.
 *                    bios::NotFound - if specified asset element was not found.
 *
 * \param url              - the connection to database.
 * \param asset_element_id - the id of the asset_element.
 *
 * \return device_discovered_id - of the device connected with the 
 *                                asset_element.
 */
m_dvc_id_t convert_asset_to_monitor(const char* url, 
                a_elmnt_id_t asset_element_id);

/**
 * \brief This function looks for an asset_element form asset part connected 
 * with the specified device_discovered from monitor part.
 *
 * Throws exceptions: bios::NotFound - in case apropriate asset element 
 *                                     was not found.
 *                    bios::InternalDBError - in case of any database errors.
 *
 * \param url                  - the connection to database.
 * \param device_discovered_id - the id of the device_discovered.
 *
 * \return asset_element_id - id of the asset_element connected with the 
 *                                device_discovered.
 */
a_elmnt_id_t convert_monitor_to_asset(const char* url, 
                    m_dvc_id_t discovered_device_id);

#endif // SRC_PERSIST_DBHELPERS_H_
