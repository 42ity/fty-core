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
#include "compute_msg.h"
#include "dbtypes.h"

#include "assettopology.h"

#define DEVICE_TYPE_EPDU 3
#define DEVICE_TYPE_PDU 4
#define DEVICE_TYPE_UPS 1
#define DEVICE_TYPE_SERVER 5

// ===========================================================================
// Helper types and functions
// ===========================================================================

/**
 * \brief Type represents a structure of unique power sources for IT devices
 * that are in some rack.
 * 
 * First  -- a set of all ePDU power sources.
 * Second -- a set of all UPS power sources.
 * Third  -- a set of all IT devices (to get info directly).
 */
typedef 
        std::tuple < std::set < device_info_t >, 
                     std::set < device_info_t >, 
                     std::set < device_info_t>  
                    >
        power_sources_t;


typedef struct _rack_power_t {
    m_msrmnt_value_t power;             //! total power with scale -1, so 2000 == 200W
    m_msrmnt_scale_t   scale;   //! scale of result
    uint8_t  quality;           //! quality of total power ^ (0 - no results found, 100 all results found)
    std::set<a_elmnt_id_t>  missed; //! devices not found in DB
} rack_power_t;

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


//TODO move device_info_t to map
/**
 * \brief Extracts power sources from power topology and sort them by device 
 * type into three cathegories.
 *
 * \param url            - connection to a database.
 * \param power_topology - a power topology represented as a pair (set of 
 *                            devices + set of powelinks).
 * \param start_device   - a device we look sources for.
 *
 * \result a tuple:
 *              first  - set of epdu.
 *              second - set of ups.
 *              third  - set of devices.
 */
power_sources_t
    extract_power_sources ( const char* url,
          std::pair < std::set < device_info_t >, 
                      std::set < powerlink_info_t > > power_topology, 
          device_info_t start_device );

/**
 * \brief Selects devices from database that are placed in the specified rack.
 *
 * throw bios::InternalDBError;
 *
 * \param url     - connection to database.
 * \param rack_id - id of the rack we look devices belong to.
 *
 * \return a set of devices placed in the rack.
 */
std::set <device_info_t> select_rack_devices(const char* url, 
                                             a_elmnt_id_t rack_id);

/**
 * \brief For a specified asset element selects is type.
 *
 * throw bios::InternalDBError;
 *
 * \param url              - connection to database.
 * \param asset_element_id - id of the asset element.
 *
 * \return 0  - if nothing was found.
 *         id - type id of found asset.
 */
a_elmnt_tp_id_t select_element_type (const char* url, 
                                     a_elmnt_id_t asset_element_id);


/**
 * \brief For every PSU (power supply unit) of every device selects 
 * a power device.
 *
 * \param url - connection to database.
 * \param rackdevices - a set of all devices contained in some rack.
 *
 * \return sources devided by its types.
 */
power_sources_t choose_power_sources (const char* url, 
                                   std::set <device_info_t> rack_devices);


/**
 * \brief Converts a c-string to double and indicate possible errors
 *
 * Changes a global variable errno if value is out of range
 *
 * \param value_str - a number in a string format
 * \param value     - an output parameter for double value
 *
 * \result 0 - success
 *         -1 a NULL pointer was recieved as an input string
 *         -2 a srting is a number, but  is out of the range (1.7E +/- 308)
 *         -3 it is not a number
 */
int convert_str_to_double (const char* value_str, double *value);


void compute_result_value_set (zhash_t *results, double value);


int compute_result_value_get (zhash_t *results, double *value);


void compute_result_value_set (zhash_t *results, m_msrmnt_value_t value);


int compute_result_value_get (zhash_t *results, m_msrmnt_value_t *value);

void compute_result_scale_set (zhash_t *results, m_msrmnt_scale_t scale);

int compute_result_scale_get (zhash_t *results, m_msrmnt_scale_t *scale);

zmsg_t* calc_total_rack_power (const char *url, a_elmnt_id_t rack_element_id);

//! \brief compute total rack power V1
//
// FIXME: leave three arguments - one per device type, maybe in the future we'll use it, or change
//
// \param upses - list of ups'es
// \param epdus - list of epdu's
// \param devs - list of devices
// \param max_age - maximum age we'd like to take into account in secs
// \return rc_power_t - total power, quality of metric and list of id's, which were requested, but missed

rack_power_t
compute_total_rack_power_v1(
        const char* url,
        const std::set<device_info_t>& upses,
        const std::set<device_info_t>& epdus,
        const std::set<device_info_t>& devs,
        uint32_t max_age);

inline
rack_power_t
compute_total_rack_power_v1(
        const char* url,
        const power_sources_t& s,
        uint32_t max_age) {

    return compute_total_rack_power_v1(
            url,
            std::get<1>(s),
            std::get<0>(s),
            std::get<2>(s),
            max_age
            );
}

#endif //SRC_PERSIST_CALC_POWER_H_
