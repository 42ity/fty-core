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
 * \file ymsg-asset.cc
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \author Tomas Halman <TomasHalman@Eaton.com>
 * \author Michal Vyskocil <MichalVyskocil@Eaton.com>
 * \brief Not yet documented file
 */
#include <string>

#include "assetcrud.h"

#include "log.h"
#include "cleanup.h"
#include "defs.h"
#include "agents.h"
#include "dbpath.h"
#include "bios_agent.h"
#include "utils.h"
#include "utils_ymsg.h"
#include "utils_app.h"
#include <cxxtools/split.h>
#include "db/assets.h"

namespace persist {

void
    process_get_asset_extra
        (ymsg_t** out, char** out_subj,
         ymsg_t* in, const char* in_subj)
{
    if ( !in || !out )
        return;
    LOG_START;
    *out_subj = strdup (in_subj);
    char *name = NULL;
    int8_t operation;
    int rv = bios_asset_extra_extract (in, &name, NULL, NULL, NULL, NULL, NULL, NULL, &operation);
    if ( rv == 0 )
    {
        try{
            tntdb::Connection conn = tntdb::connectCached(url);
            auto element = select_asset_element_by_name(conn, name);
            if ( element.status )
            {
                db_reply <std::map <std::string, std::pair<std::string, bool> >> ext_attr =
                    persist::select_ext_attributes (conn, element.item.id);
                zhash_t *ext_attributes = zhash_new();
                zhash_autofree(ext_attributes);
                for (auto &m : ext_attr.item )
                {
                    zhash_insert (ext_attributes, m.first.c_str(), (char*)m.second.first.c_str());
                }
                *out = bios_asset_extra_encode_response (element.item.name.c_str(),
                    &ext_attributes, element.item.type_id, element.item.subtype_id,
                    element.item.parent_id, element.item.status.c_str(),
                    element.item.priority, operation);
                ymsg_set_status (*out, true);
            }
            else
            {
                log_error ("Setting ASSET EXTRA reply for %s failed", name);
            }
        }
        catch(const std::exception &e) {
            LOG_END_ABNORMAL(e);
            ymsg_set_status (*out, false);
        }
    }
    else
    {
        log_error ("unable to get name of requested device, ignore the message");
    }
    FREE0(name);
    LOG_END;
}

} // namespace
