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
#include <ctime>

#include "common_msg.h"
#include "compute_msg.h"
#include "dbtypes.h"

#include "assettopology.h"

// TODO deal with hardcoded values
#define DEVICE_TYPE_EPDU 3
#define DEVICE_TYPE_PDU 4
#define DEVICE_TYPE_UPS 1
#define DEVICE_TYPE_SERVER 5


// ===========================================================================
// Helper types
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
        std::tuple < std::set< device_info_t >, 
                     std::set< device_info_t >, 
                     std::set< device_info_t >  
                    >
        power_sources_t;


/**
 * \brief A structure that represents information about total rack power
 *
 * result_total_power = power * 10 ^scale.
 */
typedef struct _rack_power_t {
    m_msrmnt_value_t           power;   //!< total power with scale (see below)
    m_msrmnt_scale_t           scale;   //!< scale of result
    uint8_t                    quality; //!< quality of total power (0 - no results found, 100 all results found)
    std::set < a_elmnt_id_t >  missed;  //!< devices where measurements not found in DB
    time_t                     date_start;
    time_t                     date_end;
} rack_power_t;


// ===========================================================================
// Main functions
// ===========================================================================


/**
 * \brief Calculate a total power of datacenter.
 *
 * Algorithm: summ up total rack powers of all racks contained in DC.
 *
 * \param url           - a connection to database.
 * \param dc_element_id - an asset element id of datacenter.
 *
 * \return encoded COMMON_MSF_FAIL or COMPUTE_MSG_RETURN
 */
zmsg_t* calc_total_dc_power (const char *url, a_elmnt_id_t dc_element_id);


/**
 * \brief Calculates a total rack power for a specified rack.
 *
 * Algorihm: divices had a priority. If there is no measurements found about 
 * device take in account a nearest power source.
 *
 * \param url             - connection to database.
 * \param rack_element_id - an id of the rack.
 *
 * \return encoded COMMON_MSF_FAIL or COMPUTE_MSG_RETURN.
 */
zmsg_t* calc_total_rack_power (const char *url, a_elmnt_id_t rack_element_id);


// ===========================================================================
// Computation functions
// ===========================================================================


/**
 * \brief compute total rack power V1
 * TODO: ask for id_key/id_subkey, 
 * see measurement_id_t nut_get_measurement_id(const std::string &name) 
 * TODO: quality computation - to be defined, leave with 255
 * 
 * \param url          - connection to database.
 * \param rack_devices - list of devices contained in a rack
 * \param date_start   - first date
 * \param date_end     - second date
 *
 * \return rc_power_t - total power, quality of metric and list of id's, 
 *                         which were requested, but missed
 * 
 * \raises tntdb::TypeError if date_start, date_end are not valid iso times!
 */
rack_power_t
compute_total_rack_power_v1(
        const char *url,
        const std::set <device_info_t> &rack_devices,
        time_t date_start,
        time_t date_end
        );

// helper function, to be removed with v1 calculation
rack_power_t
compute_total_rack_power_v1(
        const char *url,
        const std::set <device_info_t> &rack_devices,
        uint32_t max_age
        );

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
// Helper functions: deal with topology
// ===========================================================================


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
 * \brief Selects a type of specified asset element from database.
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
 * a power source device.
 *
 * \param url         - connection to database.
 * \param rackdevices - a set of all devices contained in some rack.
 *
 * \return sources devided by its types.
 */
power_sources_t choose_power_sources (const char* url, 
                                   std::set <device_info_t> rack_devices);


/**
 * \brief Find ids of racks in location topology frame.
 *
 * \param frame          - frame to process.
 * \param parent_type_id - type id of the parant in topology.
 *
 * \result set of rack ids contained in frame.
 */
std::set < a_elmnt_id_t > find_racks (zframe_t* frame, 
                                                m_dvc_tp_id_t parent_type_id);


// ===========================================================================
// Functions for work with compute msg
// ===========================================================================


/**
 * \brief Converts a c-string to double and indicate possible errors
 *
 * Changes a global variable errno if value is out of range
 *
 * \param value_str - a number in a string format
 * \param value     - an output parameter for double value
 *
 * \result  0 - success
 *         -1 - a NULL pointer was recieved as an input string
 *         -2 - a srting is a number, but  is out of the range (1.7E +/- 308)
 *         -3 - it is not a number
 */
int convert_str_to_double (const char* value_str, double *value);

/**
 * \brief Inserts a double value into hash.
 *
 * Usage: fill a compute_msg_return message.
 *
 * Value insetred with key "value_d"
 *
 * For extracting use compute_result_value_get.
 *
 * \param results - a hash
 * \param value   - a value to be inserted
 */
void compute_result_value_set (zhash_t *results, double value);


/**
 * \brief Take a double value from hash.
 *
 * A value associated with key "value_d".
 * 
 * Usage: unpack a compute_msg_return message.
 *
 * For setting use compute_result_value_set.
 *
 * \param results - a hash.
 * \param value   - an output parameter, where value would be selected.
 *
 * \return errorcode: 0 is ok otherwise with errors.
 */
int compute_result_value_get (zhash_t *results, double *value);


/**
 * \brief Inserts a integer value into hash.
 *
 * Usage: fill a compute_msg_return message.
 *
 * Value insetred with key "value"
 *
 * For extracting use compute_result_value_get.
 *
 * \param results - a hash.
 * \param value   - a value to be inserted.
 */
void compute_result_value_set (zhash_t *results, m_msrmnt_value_t value);


/**
 * \brief Take a integer value from hash.
 *
 * A value associated with key "value".
 * 
 * Usage: unpack a compute_msg_return message.
 *
 * For setting use compute_result_value_set.
 *
 * \param results - a hash.
 * \param value   - an output parameter, where value would be selected.
 *
 * \return errorcode: 0 is ok otherwise with errors.
 */
int compute_result_value_get (zhash_t *results, m_msrmnt_value_t *value);


/**
 * \brief Inserts a scale into hash.
 *
 * Usage: fill a compute_msg_return message.
 *
 * Value insetred with key "scale"
 *
 * For extracting use compute_result_scale_get.
 *
 * \param results - a hash.
 * \param scale   - a scale to be inserted.
 */
void compute_result_scale_set (zhash_t *results, m_msrmnt_scale_t scale);


/**
 * \brief Take a scale from hash.
 *
 * A value associated with key "scale".
 * 
 * Usage: unpack a compute_msg_return message.
 *
 * For setting use compute_result_scale_set.
 *
 * \param results - a hash.
 * \param value   - an output parameter, where scale would be selected.
 *
 * \return errorcode: 0 is ok otherwise with errors.
 */
int compute_result_scale_get (zhash_t *results, m_msrmnt_scale_t *scale);


/**
 * \brief Inserts a number ids for which measurements are missing into hash.
 *
 * Usage: fill a compute_msg_return message.
 *
 * Value insetred with key "num_missed"
 *
 * For extracting use compute_result_num_missed_get.
 *
 * \param results    - a hash.
 * \param num_missed - a number ids for which measurements are missing
 */
void compute_result_num_missed_set (zhash_t *results, 
                                                    a_elmnt_id_t num_missed);

/**
 * \brief Take a number ids for which measurements are missing from hash.
 *
 * A value associated with key "num_missed".
 * 
 * Usage: unpack a compute_msg_return message.
 *
 * For setting use compute_result_num_missed_set.
 *
 * \param results - a hash.
 * \param value   - an output parameter, where number ids for which 
 *                  measurements are missing would be selected.
 *
 * \return errorcode: 0 is ok otherwise with errors.
 */
int compute_result_num_missed_get (zhash_t *results, 
                                                    a_elmnt_id_t *num_missed);

#endif //SRC_PERSIST_CALC_POWER_H_
