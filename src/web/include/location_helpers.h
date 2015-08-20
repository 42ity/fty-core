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
 * \file location_helpers.h
 * \author Karol Hrdina
 * \author Michal Hrusecky
 * \brief Not yet documented file
 */
#ifndef SRC_WEB_INCLUDE_LOCATION_HELPERS
#define SRC_WEB_INCLUDE_LOCATION_HELPERS

#include <string>
#include <czmq.h>
#include "asset_msg.h"

int element_id (const std::string& from, int& element_id);
int asset (const std::string& from);
int asset_location_r(asset_msg_t** asset_msg, std::string& json);

#endif // SRC_WEB_INCLUDE_LOCATION_HELPERS

