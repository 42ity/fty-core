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
 * \file data.h
 * \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \author Michal Vyskocil <MichalVyskocil@Eaton.com>
 * \brief Not yet documented file
 */
#ifndef SRC_SHARED_DATA_H
#define SRC_SHARED_DATA_H

#include <string>
#include <map>

#include "dbhelpers.h"
#include "common_msg.h"
#include "db/assets.h"

/**
 * \brief extract error, msg and HTTP error code from common_msg instance
 *
 */
void common_msg_to_rest_error(common_msg_t* cm_msg, std::string& error, std::string& msg, unsigned* code);

class asset_manager {
    public:
        // new functionality
        db_reply <db_web_element_t>  get_item1(const std::string &id);
        db_reply <std::map <uint32_t, std::string> > get_items1(
            const std::string &typeName,
            const std::string &subtypeName);

        // to support old style
        db_reply <db_web_element_t>  get_item1(const std::string &id, const std::string &type);

        db_reply_t
            delete_item(
                const std::string &id,
                db_a_elmnt_t &element_info);
};

class measures_manager {
    public:
        std::string map_names(std::string name);
        std::string map_values(std::string name, std::string value);
        std::string apply_scale(const std::string &val, const std::string &scale);
        int
            get_last_10minute_measurement(
                const std::string &source,
                const std::string &device_name,
                m_msrmnt_value_t  &value,
                m_msrmnt_scale_t  &scale);
};


#endif // SRC_SHARED_DATA_H
