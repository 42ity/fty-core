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

#ifndef SRC_SHARED_AUGTOOL_H
//! Guard
#define SRC_SHARED_AUGTOOL_H

/*!
 * \file augtool.h
 * \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 * \brief Not yet documented file
 */
#include <string>
#include <functional>

/**
 * \brief Helper function to parse output of augtool
 *
 * If there is more than two lines, omits first and the last one. Returns all
 * values concatenated using settings in optional parameters.
 *
 * @param key_value if true each line is expected in form 'key = value' and only value is outputed
 * @param sep used to separate individual values
 * @param filter values for which it returns true are omitted
 *
 */
std::string augtool_out(const std::string in,
                        bool key_value = true,
                        std::string sep = "",
                        std::function<bool(std::string)> filter = 
                            [](const std::string) -> bool { return false; } );

#endif // SRC_SHARED_AUGTOOL_H
