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

#include <czmq.h>
#include <string>

#include "defs.h"
#include "asset_types.h"
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
        db_reply <std::map <uint32_t, std::string> > get_items1(const std::string &typeName);
        
        // to support old style
        db_reply <db_web_element_t>  get_item1(const std::string &id, const std::string &type);

        static byte type_to_byte(std::string type);
        static std::string byte_to_type(byte type);
};

class measures_manager {
    public:
        std::string map_names(std::string name);
        std::string map_values(std::string name, std::string value);
        std::string apply_scale(const std::string &val, const std::string &scale);
};

class ui_props_manager {
    public:
        int get(std::string &result);
        int put(const std::string &properties, std::string &result);
    private:
        std::string agent_name(void);
};

#endif // SRC_SHARED_DATA_H
