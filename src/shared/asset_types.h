/*
 *
 * Copyright (C) 2015 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*!
 * \file asset_types.h
 * \author Tomas Halman <TomasHalman@Eaton.com>
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 * \brief Not yet documented file
 */
#ifndef SRC_SHARED_ASSET_TYPES_H
#define SRC_SHARED_ASSET_TYPES_H

#include <string>

#include "dbtypes.h"

namespace persist {

enum asset_type {
    TUNKNOWN     = 0,
    GROUP       = 1,
    DATACENTER  = 2,
    ROOM        = 3,
    ROW         = 4,
    RACK        = 5,
    DEVICE      = 6
};

enum asset_subtype {
    SUNKNOWN = 0,
    UPS = 1,
    GENSET,
    EPDU,
    PDU,
    SERVER,
    FEED,
    STS,
    SWITCH,
    STORAGE,
    N_A = 10
};

enum asset_operation {
    INSERT = 1,
    DELETE,
    UPDATE,
    GET,
    RETIRE
};

a_elmnt_tp_id_t
    type_to_typeid
        (const std::string &type);

std::string
    typeid_to_type
        (a_elmnt_tp_id_t type_id);

a_elmnt_stp_id_t
    subtype_to_subtypeid
        (const std::string &subtype);

std::string
    subtypeid_to_subtype
        (a_elmnt_tp_id_t subtype_id);

};

#endif //SRC_SHARED_ASSET_TYPES_H
