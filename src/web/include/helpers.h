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
 * \file   helpers.h
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \brief  
 */
#ifndef SRC_WEB_INCLUDE_HELPERS_H_
#define SRC_WEB_INCLUDE_HELPERS_H_

#include <string>
#include "utils_web.h"

/*!
 \brief Perform error checking and extraction of element identifier from std::string

 \param[in]     param_name      name of the parameter from rest api call
 \param[in]     param_value     value of the parameter 
 \param[out]    element_id      extracted element identifier
 \param[out]    errors          errors structure for storing conversion errors
 \return
    true on success, element_id is assigned the converted element identifier
    false on failure, errors are updated (exactly one item is added to structure)
*/
bool 
check_element_identifier (const char *param_name, const std::string& param_value, uint32_t& element_id, http_errors_t& errors);

#endif // SRC_WEB_INCLUDE_HELPERS_H_

