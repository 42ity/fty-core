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

/*! \file csv.h
    \brief Read values from CSV files
    \author Michal Vyskocil <MichalVyskocil@Eaton.com>

Example:

    std::stringstream buf;

    buf << "Name, Type, Group.1, group.2,description\n";
    buf << "RACK-01, rack,GR-01, GR-02,just my dc\n";
    buf << "RACK-02, rack,GR-01, GR-02,just my rack\n";

    std::vector<std::vector<std::string> > data;
    cxxtools::CsvDeserializer deserializer(buf);
    deserializer.delimiter(',');
    deserializer.readTitle(false);
    deserializer.deserialize(data);

    shared::CsvMap cm{data};
    cm.deserialize();

    assert (cm.get(0, "nAMe") == "Name");
    assert (cm.get(1, " name") == "RACK-01");
*/

#ifndef SRC_SHARED_CSV_H
#define SRC_SHARED_CSV_H

#include <vector>
#include <string>
#include <map>
#include <cstdint>

#include <cxxtools/csvdeserializer.h>

namespace shared {
    /**
     * \class CsvMap
     *
     * \brief Provide map-like interface to result of csv file
     *
     * This class does provide map-like interface to tabular data
     * from csv file, so you can refer to columns using name of
     * field
     *
     */
    class CsvMap {

        public:
            typedef std::vector<std::vector<std::string> > Data;
            typedef std::vector<std::vector<cxxtools::String> > CxxData;

          /**
           * \brief Creates new CsvMap instance with data inside
           */
            CsvMap(const Data& data) :
                _data{data},
                _title_to_index{}
            {};

           /**
           * \brief Creates new CsvMap instance with data inside
           */
            CsvMap(const CxxData& data);

            /**
             * \brief deserialize provided data, inicialize map of row title to index
             *
             * \throws std::invalid_argument if csv contain multiple title with the
             *         same name
             */
            void deserialize();

            /**
             * \brief return the content on row with the given title name
             *
             * \throws std::out_of_range if row_i > data.size() or title_name is not known
             */
            const std::string& get(size_t row_i, const std::string& title_name) const;

            /**
             * \brief return the content on row with the given title name striped and in lower case
             *
             * \throws std::out_of_range if row_i > data.size() or title_name is not known
             */
            std::string get_strip(size_t row_i, const std::string& title_name) const;

            /**
             * \brief return number of rows
             */
            size_t rows() const {
                return _data.size();
            }

            /**
             * \brief return number of columns
             */
            size_t cols() const {
                return _data[0].size();
            }

            /**
             * \brief return if given title exists
             */
            bool hasTitle(const std::string& title_name) const;

            /**
             * \brief get copy of titles
             */
            std::set<std::string> getTitles() const;

        private:
            Data _data;
            std::map<std::string, size_t> _title_to_index;
    };

//TODO: does not belongs to csv, move somewhere else
void skip_utf8_BOM (std::istream& i);

/**
 * \brief find the delimiter used in csv file
 *
 * \param i istream, which is analyzed
 * \param max_pos specifies how many bytes should be investigated before
 *                \x0 is returned. Defaults to 60.
 *
 * \return ';' or '\t' or '\x0' if nothing found in first \max_pos bytes
 */
char
findDelimiter(
        std::istream& i,
        std::size_t max_pos = 60);


/**
 *  \brief read the data from istream
 *
 *  \param[in] input stream
 *  \return CsvMap instance
 *  \throws invalid_argument if delimiter was not autodetected
 *          ... or various other exceptions ;-)
 */
CsvMap
CsvMap_from_istream(
        std::istream& in);

/**
 *  \brief read the data from serialization info
 *
 * In short, it can converts this
 * { "name" : "DC-1", "type" : "datacenter", "ext" : {"ext1name" : "ext1value" }}
 *
 * into that
 *
 * std::vector<std::vector<cxxtools::String>> {
 *  {"name", "type", "ext1name"},
 *  {"DC-1", "datacenter", "ext1value"}
 * }
 *
 *  \param[in] input serialization info
 *  \return CsvMap instance
 *
 *  \throws invalid_argument if typeName of si is not Object, or it does not contain strings
 *          ... or various other exceptions ;-)
 */
CsvMap
CsvMap_from_serialization_info(
        const cxxtools::SerializationInfo& si);

} //namespace shared
#endif // SRC_SHARED_CSV_H
