/*
Copyright (C) 2015 Eaton
 
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

/*! \file   calc_power.h
    \brief  Functions for calculating a total rack and DC power from database values.
    \author Alena Chernikava <alenachernikava@eaton.com>
    \author Michal Vyskocil  <michalvyskocil@eaton.com>
*/

#ifndef SRC_PERSIST_CALC_POWER_H_
#define SRC_PERSIST_CALC_POWER_H_

#include <map>
#include <vector>

#include <tntdb/connect.h>
#include "dbtypes.h"
#include "dbhelpers.h" // db_reply

#include "assettopology.h" //definition of device_info_t 

// TODO deal with hardcoded values
#define DEVICE_TYPE_EPDU 3
#define DEVICE_TYPE_PDU 4
#define DEVICE_TYPE_UPS 1
#define DEVICE_TYPE_SERVER 5


// ===========================================================================
// Device type check functions
// ===========================================================================


/**
 * \brief Checks the type of device.
 *
 * \param device - a device to be checked.
 *
 * \return true  if it is an epdu.
 *         false if it is not an epdu.
 */
bool is_epdu (const device_info_t &device);


/**
 * \brief Checks the type of device.
 *
 * \param device - a device to be checked.
 *
 * \return true  if it is a pdu.
 *         false if it is not a pdu.
 */
bool is_pdu (const device_info_t &device);


/**
 * \brief Checks the type of device.
 *
 * \param device - a device to be checked.
 *
 * \return true  if it is a ups.
 *         false if it is not a ups.
 */
bool is_ups (const device_info_t &device);


/**
 * \brief Checks the type of device.
 *
 * \param device - a device to be checked.
 *
 * \return true  if it is an IT device.
 *         false if it is not an IT device.
 */
bool is_it_device (const device_info_t &device);


// ===========================================================================
// Functions that find power sources
// ===========================================================================

/**
 * \brief For every rack analyses its power topology and
 *        for each rack returns a list of power devices
 *        that belong to "input power".
 * 
 * Because this function is intent to support an agent (but agent
 * has no idea about ids) that listens to the stream, so
 * an output is supposed to be names of devices.
 *
 * \param conn - a connection to the database
 *
 * \return in case of success: status = 1,
 *                             item is set to be a map of rack names
 *                                  onto its power sources
 *         in case of fail:    status = 0,
 *                             errtype is set,
 *                             errsubtype is set,
 *                             msg is set
 */
db_reply <std::map<std::string, std::vector<std::string> > >
    select_devices_total_power_racks
        (tntdb::Connection  &conn);


/**
 * \brief For every dc analyses its power topology and
 *        for each dc returns a list of power devices
 *        that belong to "input power".
 * 
 * Because this function is intent to support an agent (but agent
 * has no idea about ids) that listens to the stream, so
 * an output is supposed to be names of devices.
 *
 * \param conn - a connection to the database
 *
 * \return in case of success: status = 1,
 *                             item is set to be a map of dc names
 *                                  onto its power sources
 *         in case of fail:    status = 0,
 *                             errtype is set,
 *                             errsubtype is set,
 *                             msg is set
 */
db_reply <std::map<std::string, std::vector<std::string> > >
    select_devices_total_power_dcs
        (tntdb::Connection  &conn);

#endif //SRC_PERSIST_CALC_POWER_H_
