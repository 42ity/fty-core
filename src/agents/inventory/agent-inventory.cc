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
#include "bios_agent.h"
#include "common_msg.h"
#include "defs.h"
#include "log.h"
#include "dbpath.h"
#include "persistencelogic.h"
#include "cleanup.h"

// it is a persistence agent, so it has a right to comunicate with DB directly

/**
 * \brief main inventory actor loop.
 *
 * It waits for a message from measurements stream with subject "inventory.*"
 */
int main (int argc, char *argv[])
{
    // ASSUMPTION:
    //  this agent is already registred in DB
    log_open();
    log_info ("persistence.inventory started");
    
    // Basic settings
    if ( argc > 3 )
    {
        printf ("syntax: db-inventory [ <endpoint> | <endpoint> <mysql:db="
                "bios;user=bios;password=test> ]\n");
        return 1;
    }
    // TODO read address of malamute from configuration file
    const char *addr = (argc == 1) ? "ipc://@/malamute" : argv[1];
    if ( argc > 2 )
    {
        url = argv[2];
    }

    // Create an agent
    _scoped_bios_agent_t *agent = bios_agent_new(addr, "persistenceinventory");
    if ( !agent ) {
        log_error ("db-inventory: error bios_agent_new");
        return 1;
    }
    // listen on inventory messages
    bios_agent_set_consumer (agent, bios_get_stream_main (), "^inventory@.+"); 

    while ( !zsys_interrupted )
    {
        // ASSUMPTION:
        //  messages will never be lost, so no need to manage a queue
        //  of sended messages
        auto new_msg = bios_agent_recv (agent);
        
        if ( new_msg == NULL )
            continue;
        
        log_info ("Command is '%s'", bios_agent_command (agent));

        // from stream we can recieve only one type of messages
        if ( streq (bios_agent_command (agent), "STREAM DELIVER") )
        // TODO use some constants
        {           
            persist::process_inventory (&new_msg);
            // TODO process ret_value
        }
        else
        {
            // for now we don't support any direct messages
            // TODO: should we log whole message ?
            log_error ("recieved non STREAM deliver, ignore it");
        }
        // we should destroy the message
        ymsg_destroy (&new_msg);
    }

    bios_agent_destroy (&agent);
    log_info ("persistence.inventory ended");
    log_close();
    return 0; // TODO getrid of constants
}
