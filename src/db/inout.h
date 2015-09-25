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

struct bad_request_document : std::invalid_argument {
    explicit bad_request_document(const std::string& msg):
        std::invalid_argument(msg)
    {}
};

struct request_param_required : std::invalid_argument {
    explicit request_param_required(const std::string& msg):
        std::invalid_argument(msg)
    {}
};

struct element_not_found : std::invalid_argument {
    explicit element_not_found(const std::string& msg):
        std::invalid_argument(msg)
    {}
};

struct action_forbidden : std::invalid_argument {
    explicit action_forbidden(const std::string& msg):
        std::invalid_argument(msg)
    {}
};

struct request_bad_param : std::invalid_argument {
    explicit request_bad_param(
            const std::string& name,
            const std::string& got,
            const std::string& expected):
        std::invalid_argument("bad parameter"),
        name(name),
        got(got),
        expected(expected)
    {
        std::stringstream s;
        // copy&paste from _errors
        s << "Parameter '" << name << "' has bad value. Received '" << got << "'. Expected '"<< expected << "'";
        _what = s.str();
    };

    std::string name;
    std::string got;
    std::string expected;
    std::string _what;

    virtual const char* what() const throw()
    {
        return _what.c_str();
    }
};

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
         std::vector <std::pair<db_a_elmnt_t,persist::asset_operation>> &okRows,
         std::map <int, std::string> &failRows);

/*
 * \brief Processes a csv map
 *
 * Resuls are written in DB and into log.
 *
 * \param cm[in]     - an input csv map
 * \param okRows[out]   - a list of short information about inserted rows
 * \param failRows[out] - a list of rejected rows with the message
 */
void
    load_asset_csv
        (const shared::CsvMap& cm,
         std::vector <std::pair<db_a_elmnt_t,persist::asset_operation>> &okRows,
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
