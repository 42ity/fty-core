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
 * \file agent-inventory.cc
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \brief Not yet documented file
 */
#include <string>
#include <ctime>
#include <fty_proto.h>
#include <tntdb/connect.h>
#include <tntdb/error.h>

#include "bios_agent.h"
#include "defs.h"
#include "log.h"
#include "dbpath.h"
#include "assetcrud.h"
#include "cleanup.h"

int main (int argc, char *argv[])
{
    log_open();
    log_info ("%s started.", BIOS_AGENT_NAME_DB_INVENTORY);
    
    // Basic settings
    if (argc > 3) {
        printf ("syntax: db-inventory [ <endpoint> | <endpoint> <mysql:db="
                "bios;user=bios;password=test> ]\n");
        return EXIT_FAILURE;
    }
    const char *addr = (argc == 1) ? MLM_ENDPOINT : argv[1];
    if (argc > 2) {
        url = argv[2];
    }

    // Create an agent
    mlm_client_t *agent = mlm_client_new ();
    if (!agent) {
        log_error ("mlm_client_new () failed.");
        return EXIT_FAILURE;
    }

    int rv = mlm_client_connect (agent, MLM_ENDPOINT, 1000, BIOS_AGENT_NAME_DB_INVENTORY);
    if (rv != 0) {
        log_error ("mlm_client_connect () failed.");
        mlm_client_destroy (&agent);
        return EXIT_FAILURE;
    }

    // listen on inventory messages
    rv = mlm_client_set_consumer (agent, FTY_PROTO_STREAM_ASSETS, ".*");
    if (rv == -1) {
        log_error ("mlm_client_set_consumer () failed");
        mlm_client_destroy (&agent);
        return EXIT_FAILURE;
    }

    while (!zsys_interrupted) {

        zmsg_t *new_msg = mlm_client_recv (agent);
        if (!new_msg)
              break;
	
        if (!is_fty_proto (new_msg)) {
             log_warning ("not a bios proto message sender == '%s', subject == '%s', command = '%s'",
                     mlm_client_sender (agent), mlm_client_subject (agent), mlm_client_command (agent));
             zmsg_destroy (&new_msg);
             continue;
        }
          
        fty_proto_t *proto = fty_proto_decode (&new_msg);
        if (!proto) {
            log_critical ("fty_proto_decode () failed");
            continue;
        }
	
        const char *device_name = fty_proto_name (proto);
        assert (device_name);
        zhash_t *ext = fty_proto_ext (proto);
        assert (ext);
	    const char *operation = fty_proto_operation(proto);
        assert (operation);

        if (!streq (operation, "inventory")) {
            fty_proto_destroy (&proto);
            continue;  
        }
	
        try {
            tntdb::Connection conn = tntdb::connectCached (url);
            process_insert_inventory (
                    conn,
                    device_name,
                    ext);
        }
        catch (const std::exception& e) {
            fty_proto_destroy (&proto);
            zsys_error ("tntdb::connectCached () failed: %s", e.what ());
            break;
        }
        fty_proto_destroy (&proto);
    }

    mlm_client_destroy (&agent);
    log_info ("%s finished.", BIOS_AGENT_NAME_DB_INVENTORY);
    return EXIT_SUCCESS;
}
