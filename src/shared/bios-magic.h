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
 * \file bios-magic.h
 * \author Michal Vyskocil
 * \author Alena Chernikava
 * \brief Not yet documented file
 */
#ifndef SRC_WEB_INCLUDE_ENC_H
#define SRC_WEB_INCLUDE_ENC_H

#include <string>
#include <utility>
#include <iostream>

namespace shared {
/*
 *  \brief return file (type, encoding)
 *
 *  \throws std::logic_error if magic cookie can't be loaded,
 *          database can't be loaded or file can't be analyzed
 */
std::pair<std::string, std::string>
file_type_encoding(const char* path);

// quick and dirty convert function, NEVER use it!!
void
convert_file(
        std::string::const_iterator begin,
        std::string::const_iterator end,
        std::string& out);

} //namespace shared

#endif //SRC_WEB_INCLUDE_ENC_H
