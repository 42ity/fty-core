/*
Copyright (C) 2015 Eaton

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

/*! \file   inout.h
    \brief  Import/Export of csv
    \author Michal Vyskocil <MichalVyskocil@Eaton.com>
    \author Alena Chernikava <AlenaChernikava@Eaton.com>
*/
#ifndef SRC_DB_INOUT_H
#define SRC_DB_INOUT_H

#include <iostream>
#include <vector>
#include <map>

#include "dbhelpers.h"
namespace persist {

/*
 * \brief Converts the string priority to number
 *
 * get_priority("0") -> 0
 * get_priority("P3") -> 3
 * get_priority("X4") -> 4
 * get_priority("777") -> 5
 *
 * \param input[s]     - string with content
 * \return numeric value of priority or 5 if not detected
 *
 */
int
get_priority(
        const std::string& s);

/*
 * \brief Converts the string yes/no to boolean value
 *
 * get_priority("yEs") -> true
 * get_priority("No") -> false
 * get_priority("spam") -> false
 *
 * \param input[s]     - string with content
 * \return boolean value
 *
 */
bool
    get_business_critical(
            const std::string& bs_critical);
/*
 * \brief Processes a csv file
 *
 * Resuls are written in DB and into log.
 *
 * \param input[in]     - an input file
 * \param okRows[out]   - a list of short information about inserted rows
 * \param failRows[out] - a list of rejected rows with the message
 */
void
    load_asset_csv
        (std::istream& input,
         std::vector <db_a_elmnt_t> &okRows,
         std::map <int, std::string> &failRows);

/** \brief export csv file and write result to output stream
 *
 * \param[out] out - reference to standard output stream to which content will be written
 */
void
    export_asset_csv
        (std::ostream& out);
}
#endif
