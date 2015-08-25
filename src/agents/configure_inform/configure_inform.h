#include <vector>
/*
Copyright (C) 2014-2015 Eaton

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

/*! \file   configure_inform.h
    \brief  Agent body for sending configure messages

    \author Alena Chernikava <AlenaChernikava@Eaton.com>
*/

#include <string>
#include <vector>

#include "dbhelpers.h"
#include "asset_types.h"
#include "log.h"

void
    send_configure (
        const std::vector <std::pair
            <db_a_elmnt_t, asset_type::asset_operation>> &rows,
        const std::string &agent_name);

void
    send_configure (
        const db_a_elmnt_t row,
        asset_type::asset_operation action_type,
        const std::string &agent_name);

void
    send_configure (
        const std::pair<db_a_elmnt_t, asset_type::asset_operation> row,
        const std::string &agent_name);
