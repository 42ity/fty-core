/*
Copyright (C) 2014 - 2015 Eaton

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

/*! \file   agents.h
    \brief  Not yet documented file
    \author Karol Hrdina <KarolHrdina@Eaton.com>
*/

#ifndef INCLUDE_AGENTS_H__
#define INCLUDE_AGENTS_H__

#include <czmq.h>

#include "bios_export.h"
#include "ymsg.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief extract extended asset message
 *
 * \param[in]  message        - asset extended message to decode
 * \param[out] name           - name of the asset element
 * \param[out] ext_attributes - a map of extended attributes.
 * \param[out] type_id        - id of the lement type.
 * \param[out] parent_id      - id of the parent device (0 if it has no parent device).
 * \param[out] status         - status of the element
 * \param[out] priority       - priority of the elemnt.
 * \param[out] event type     - type of action with element 1(insert), 2(update), 3(delete).
 *
 * \return int  0 = success,
 *             -1 = invalid/unspecified parameter,
 *             -3 = malformed message.
 */
BIOS_EXPORT int
bios_asset_extra_extract(ymsg_t *message,
                   char **name,
                   zhash_t **ext_attributes,
                   uint32_t *type_id,
                   uint32_t *subtype_id,
                   uint32_t *parent_id,
                   char **status,
                   uint8_t *priority,
                   int8_t *event_type);


/**
 * \brief Encode extended asset message as reply
 *
 * \param[in] name           - name of the asset element
 * \param[in] ext_attributes - a map of extended attributes.
 * \param[in] type_id        - id of the lement type.
 * \param[in] parent_id      - id of the parent device (0 if it has no parent device).
 * \param[in] status         - status of the element
 * \param[in] priority       - priority of the elemnt.
 * \param[in] event type     - type of action with element 1(insert), 2(update), 3(delete).
 *
 * \return ymsg_t * or NULL if failed
 */
BIOS_EXPORT ymsg_t *
bios_asset_extra_encode_response(const char *name,
                   zhash_t **ext_attributes,
                   uint32_t type_id,
                   uint32_t subtype_id,
                   uint32_t parent_id,
                   const char* status,
                   uint8_t priority,
                   int8_t event_type);


/**
 * \brief Encode extended asset message as send
 *
 * \param[in] name           - name of the asset element
 * \param[in] ext_attributes - a map of extended attributes.
 * \param[in] type_id        - id of the lement type.
 * \param[in] parent_id      - id of the parent device (0 if it has no parent device).
 * \param[in] status         - status of the element
 * \param[in] priority       - priority of the elemnt.
 * \param[in] event type     - type of action with element 1(insert), 2(update), 3(delete).
 *
 * \return ymsg_t * or NULL if failed
 */
BIOS_EXPORT ymsg_t *
bios_asset_extra_encode(const char *name,
                   zhash_t **ext_attributes,
                   uint32_t type_id,
                   uint32_t subtype_id,
                   uint32_t parent_id,
                   const char* status,
                   uint8_t priority,
                   int8_t event_type);

#ifdef __cplusplus
}
#endif

#endif // INCLUDE_AGENTS_H__
