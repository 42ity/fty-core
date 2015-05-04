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

//! encoded message or NULL on failure
BIOS_EXPORT ymsg_t *
    bios_netmon_encode (int event, const char *interface_name, int ip_version, const char *ip_address, uint8_t prefix_length, const char *mac_address);

//! 0 on success, -1 on failure
BIOS_EXPORT int
    bios_netmon_extract (ymsg_t *self, int *event, char **interface_name, int *ip_version, char **ip_address, uint8_t *prefix_length, char **mac_address);


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
/*
 \brief Encode request message TODO
 \note You are responsible for destroying the returned encoded message
 \return encoded message on success, NULL on failure
*/
BIOS_EXPORT ymsg_t *
    bios_web_average_request_encode (int64_t start_timestamp, int64_t end_timestamp, const char *type, const char *step, uint64_t element_id, const char *source);

//!0 on success, -1 or failure
BIOS_EXPORT int
    bios_web_average_request_extract (ymsg_t *self, int64_t *start_timestamp, int64_t *end_timestamp, char **type, char **step, uint64_t *element_id, char **source);

/*!
 \brief Encode reply message TODO
 \note You are responsible for destroying the returned encoded message
 \return encoded message on success, NULL on failure
*/
BIOS_EXPORT ymsg_t *
    bios_web_average_reply_encode (const char *json);

//! 0 on success, -1 or failure
BIOS_EXPORT int
    bios_web_average_reply_extract (ymsg_t *self, char **json);

/*!
 \brief Encode request message TODO
 \note You are responsible for destroying the returned encoded message
 \note You are responsible for freeing `subject` on success
 \return Encoded message on success, NULL on failure
*/
BIOS_EXPORT ymsg_t *
    bios_db_measurements_read_request_encode (int64_t start_timestamp, int64_t end_timestamp, uint64_t element_id, const char *source, char **subject);

//! 0 on success, -1 or failure
BIOS_EXPORT int
    bios_db_measurements_read_request_extract (ymsg_t *self, int64_t *start_timestamp, int64_t  *end_timestamp, uint64_t *element_id, char **source);

/*!
 \brief Encode request message TODO
 \note You are responsible for destroying the returned encoded message
 \return Encoded message on success, NULL on failure
*/
BIOS_EXPORT ymsg_t *
    bios_db_measurements_read_reply_encode (const char *json);

//! 0 on success, -1 or failure
BIOS_EXPORT int
    bios_db_measurements_read_reply_extract (ymsg_t *self, char **json);

typedef enum {
    ALERT_STATE_UNKNOWN = -1,
    ALERT_STATE_NO_ALERT,
    ALERT_STATE_ONGOING_ALERT
} alert_state_t;

typedef enum {
    ALERT_PRIORITY_UNKNOWN = 0,
    ALERT_PRIORITY_P1,
    ALERT_PRIORITY_P2,
    ALERT_PRIORITY_P3,
    ALERT_PRIORITY_P4,
    ALERT_PRIORITY_P5
} alert_priority_t;

/**
 * \brief encode measurement message
 *
 * \param rule_name - name of rule causing alarm (something like upsonbattery@ups1.example.com) 
 * \param priority - alert priority P1 - P5
 * \param state - alert state (NO_ALERT or ONGOING_ALERT).
 * \param devices - list of devices causing alarm separated by ","
 *                  (example smoke1.example.com,smoke2.example.com)
 * \param alert_description - optional, user description
 * \param since - time, when this alarm state has been discovered.
 * \return ymsg_t * or NULL if failed
 */
BIOS_EXPORT ymsg_t *
bios_alert_encode (const char *rule_name,
                   uint8_t priority,
                   int8_t state,
                   const char *devices,
                   const char *alert_description,
                   time_t since);

/**
 * \brief extract alert message
 *
 * \param[in] ymsg - alert message to decode
 * \param[out] rule_name - alert rule name.
 * \param[out] priority - alert priority P1 - P5.
 * \param[out] alert_state - alert is triggered in the system or it is not (end of alert).
 * \param[out] devices - devices, causing this alert (comma separated list).
 * \param[out] description - optional, user alert description.
 * \param[out] since - time, when this alarm state has been discovered.
 * \return int 0 = success, -1 = invalid/unspecified parameter,
 *             -3 = some or all information in message is missing.
 *             -4 = invalid priority
 *             -5 = invalid state
 *             -6 = malformed timestamp
 *
 */
BIOS_EXPORT int
bios_alert_extract(ymsg_t *self,
                   char **rule_name,
                   uint8_t *priority,
                   int8_t *state,
                   char **devices,
                   char **description,
                   time_t *since);

BIOS_EXPORT ymsg_t *
bios_asset_encode( const char *devicename,
                   uint32_t type_id,
                   uint32_t parent_id,
                   const char* status,
                   uint32_t priority);

BIOS_EXPORT int
bios_asset_extract(ymsg_t *message,
                   char **devicename,
                   uint32_t *type_id,
                   uint32_t *parent_id,
                   char **status,
                   uint32_t *priority);


#ifdef __cplusplus
}
#endif

#endif // INCLUDE_AGENTS_H__
