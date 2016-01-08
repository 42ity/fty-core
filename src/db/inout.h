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
#include <string>
#include <map>
#include <stdexcept>

#include "asset_types.h"
#include "csv.h"

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
 * \param[in] s - a string with a content
 *
 * \return numeric value of priority or 5 if not detected
 *
 */
int
get_priority(
        const std::string& s);


/*
 * \brief process one asset
 *
 * \param[in] cm - an instance of CsvMap (you see this nice evolution, right)
 *
 * \return id of inserted/updated asset
 *
 * \throws execeptions on errors
 *
 */
std::pair<db_a_elmnt_t, persist::asset_operation>
    process_one_asset
        (const shared::CsvMap& cm);
/*
 * \brief Processes a csv file
 *
 * Resuls are written in DB and into log.
 *
 * \param[in]  input    - an input file
 * \param[out] okRows   - a list of short information about inserted rows
 * \param[out] failRows - a list of rejected rows with the message
 */
void
    load_asset_csv
        (std::istream& input,
         std::vector <std::pair<db_a_elmnt_t,persist::asset_operation>> &okRows,
         std::map <int, std::string> &failRows);

/*
 * \brief Processes a csv map
 *
 * Resuls are written in DB and into log.
 *
 * \param[in]  cm       - an input csv map
 * \param[out] okRows   - a list of short information about inserted rows
 * \param[out] failRows - a list of rejected rows with the message
 */
void
    load_asset_csv
        (const shared::CsvMap& cm,
         std::vector <std::pair<db_a_elmnt_t,persist::asset_operation>> &okRows,
         std::map <int, std::string> &failRows);

/** \brief export csv file and write result to output stream
 *
 * \param[out] out - a reference to the standard output stream to which content will be written
 */
void
    export_asset_csv
        (std::ostream& out);
}
#endif
