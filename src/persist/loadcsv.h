/*
Copyright (C) 2015 Eaton
 
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

/*! \file   loadcsv.h
    \brief  Import of csv
    \author Michal Vyskocil   <michalvyskocil@eaton.com>
            Alena  Chernikava <alenachernikava@eaton.com>
*/
#ifndef SRC_PERSIST_LOADCSV_H
#define SRC_PERSIST_LOADCSV_H

#include <vector>
#include <map>

#include "dbhelpers.h"
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

#endif
