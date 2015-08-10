#include <vector>
/*
Copyright (C) 2014-2015 Eaton

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

/*! \file configure_inform.h
    \brief Agent body for sending confidure messages

    \author Alena Chernikava <AlenaChernikava@eaton.com>
*/

#include <string>
#include <string>

#include "dbhelpers.h"
#include "log.h"

void
    send_configure (
        std::vector<db_a_elmnt_t> rows,
        uint8_t action_type,
        const std::string &agent_name);
