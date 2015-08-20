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
 * \author Tomas Halman
 * \author Alena Chernikava
 * \author Michal Hrusecky
 * \brief Not yet documented file
 */
#ifndef SRC_SHARED_ASSET_TYPES_H
#define SRC_SHARED_ASSET_TYPES_H

namespace asset_type {

enum asset_type {
    UNKNOWN     = 0,
    GROUP       = 1,
    DATACENTER  = 2,
    ROOM        = 3,
    ROW         = 4,
    RACK        = 5,
    DEVICE      = 6
};

enum asset_subtype {
    UPS = 1,
    GENSET,
    EPDU,
    PDU,
    SERVER,
    MAIN,
    STS,
    SWITCH,
    STORAGE,
    N_A = 10
};

enum asset_operation {
    INSERT = 1,
    DELETE,
    UPDATE,
    GET
};

};

#endif //SRC_SHARED_ASSET_TYPES_H
