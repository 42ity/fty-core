/*
Copyright (C) 2014 - 2015 Eaton

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

/*!
 \file   agents.h
 \brief  TODO
 \author Karol Hrdina <KarolHrdina@eaton.com>
*/

#ifndef INCLUDE_AGENTS_H__
#define INCLUDE_AGENTS_H__

#include <czmq.h>
#include <bios_export.h>

#include "ymsg.h"

#ifdef __cplusplus
extern "C" {
#endif

BIOS_EXPORT ymsg_t *
    bios_netmon_encode (int event, const char *interface_name, int ip_version, const char *ip_address, uint8_t prefix_length, const char *mac_address);


// on -1 (error) does not destroy *self_p
BIOS_EXPORT int
    bios_netmon_decode (ymsg_t **self_p, int *event, char **interface_name, int *ip_version, char **ip_address, uint8_t *prefix_length, char **mac_address);


BIOS_EXPORT ymsg_t *
    bios_inventory_encode (const char *device_name, zhash_t **ext_attributes, const char *module_name);


// on -1 (error) does not destroy *self_p
BIOS_EXPORT int
    bios_inventory_decode (ymsg_t **self_p, char **device_name, zhash_t **ext_attributes, char **module_name);

/**
 * \brief encode measurement message
 *
 * \param device_name - name of device / hostname
 * \param quantity - quantity name, for example "realpower.default"
 * \param units - quantity units, for example "W" 
 * \param value - real measurement value is value*10^scale 
 * \param scale
 * \param time - time, when measurement was done. Value -1 means use current time
 * \return ymsg_t * or NULL if failed
 */
BIOS_EXPORT ymsg_t *
    bios_measurement_encode (const char *device_name,
                             const char *quantity,
                             const char *units,
                             int32_t value,
                             int32_t scale,
                             int64_t time);


/**
 * \brief decode measurement message
 *
 * \param[in] ymsg - measurement message to decode
 * \param[out] device_name - pointer name of device/hostname. Use free to dealocate memory.
 * \param[out] quantity - quantity name, for example "realpower.default".
 *             Use free to dealocate memory.
 * \param[out] units - quantity units, for example "W". Use free to dealocate memory. 
 * \param[out] value - real measurement value is value*10^scale
 * \param[out] scale
 * \param[out] time - time, when measurement was done. Value -1 means use current time
 * \return int 0 = success, -1 = invalid/unspecified parameter, -2 = ymsg is NULL,
 *             -3 = some or all information in message is missing. In case of error
 *             ymsg is not destroyed.
 */
BIOS_EXPORT int
    bios_measurement_decode (ymsg_t **self_p,
                             char **device_name,
                             char **quantity,
                             char **units,
                             int32_t *value,
                             int32_t *scale,
                             int64_t *time);

BIOS_EXPORT ymsg_t *
    bios_web_average_request_encode (const char *start_timestamp, const char *end_timestamp, const char *type, const char *step, uint64_t element_id, const char *source);

BIOS_EXPORT int
    bios_web_average_request_decode (ymsg_t **self_p, char **start_timestamp, char **end_timestamp, char **type, char **step, uint64_t *element_id, char **source);

BIOS_EXPORT ymsg_t *
    bios_web_average_reply_encode (const char *json);

BIOS_EXPORT int
    bios_web_average_reply_decode (ymsg_t **self_p, char **json);

BIOS_EXPORT ymsg_t *
    bios_db_measurements_read_request_encode (const char *start_timestamp, const char *end_timestamp, uint64_t element_id, const char *source, char **subject);

BIOS_EXPORT int
    bios_db_measurements_read_request_decode (ymsg_t **self_p, char **start_timestamp, char **end_timestamp, uint64_t *element_id, char **source);
    
BIOS_EXPORT ymsg_t *
    bios_db_measurements_read_reply_encode (const char *json);

BIOS_EXPORT int
    bios_db_measurements_read_reply_decode (ymsg_t **self_p, char **json);

#ifdef __cplusplus
}
#endif

#endif // INCLUDE_AGENTS_H__
