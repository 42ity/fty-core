/*
Copyright (C) 2014-2015 Eaton

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

#include "agentstate.h"

#include <tntdb/row.h>
#include <tntdb/error.h>

#include "log.h"
#include "utils.h"


int
    update_agent_info(
        tntdb::Connection &conn,
        const std::string &agent_name,
        const void        *data,
        size_t             size,
        uint16_t          &affected_rows
        )
{
    LOG_START;

    try{
        tntdb::Statement st = conn.prepareCached(
            " INSERT INTO t_bios_agent_info (agent_name, info) "
            " VALUES "
            "   (:name, :info) "
            " ON DUPLICATE KEY "
            "   UPDATE "
            "       info = :info "
        );
        tntdb::Blob blobData((const char *)data, size);

        affected_rows = st.set("name", agent_name).
                           setBlob("info", blobData).
                           execute();
        log_debug ("[t_bios_agent_info]: %" PRIu16 " rows ", affected_rows);
        LOG_END;
        return 0;
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return 1;
    }
}

int
    select_agent_info(
        tntdb::Connection &conn,
        const std::string &agent_name,
        void             **data,
        size_t             &size
        )
{
    LOG_START;

    if ( data == NULL )
        return 5;
    try{
        *data = NULL;
        size = 0;
        tntdb::Statement st = conn.prepareCached(
            " SELECT "
            "   v.info "
            " FROM "
            "   v_bios_agent_info v "
            " WHERE "
            "   v.agent_name = :name "
        );

        tntdb::Row row = st.set("name", agent_name).
                            selectRow();

        tntdb::Blob myBlob;
        row[0].get(myBlob);

        size = myBlob.size();
        *data = new char[size];
        memcpy(*data, myBlob.data(), size);
        LOG_END;
        return 0;
    }
    catch (const tntdb::NotFound &e) {
        log_debug ("end: nothing was found");
        return 0;
    }
    catch (const std::exception &e) {
        LOG_END_ABNORMAL(e);
        return 1;
    }
}
