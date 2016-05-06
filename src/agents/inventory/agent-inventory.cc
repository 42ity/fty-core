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
#include "defs.h"
#include "log.h"
#include "dbpath.h"
#include "persistencelogic.h"
#include "cleanup.h"
#include <biosproto.h>

/**
 * \brief main inventory actor loop.
 *
 * Listens on 'main' stream for subject (regex) "^inventory@.+"
 */
int main (int argc, char *argv[])
{
    // ASSUMPTION:
    //  this agent is already registred in DB
    log_open();
    log_info ("%s started.", BIOS_AGENT_NAME_DB_INVENTORY);
    
    // Basic settings
    if ( argc > 3 )
    {
        printf ("syntax: db-inventory [ <endpoint> | <endpoint> <mysql:db="
                "bios;user=bios;password=test> ]\n");
        return 1;
    }
    const char *addr = (argc == 1) ? MLM_ENDPOINT : argv[1];
    if ( argc > 2 )
    {
        url = argv[2];
    }

    // Create an agent
    //_scoped_bios_agent_t *agent = bios_agent_new (addr, BIOS_AGENT_NAME_DB_INVENTORY);
    mlm_client_t *agent = mlm_client_new();
    
    if ( !agent ) {
        log_error ("mlm_client_new() failed.");
        return EXIT_FAILURE;
    }
    int rv = mlm_client_connect(agent, MLM_ENDPOINT, 1000, addr);
    if (rv != 0) {
        log_error ("mlm_client_connect () failed.");
	mlm_client_destroy (&agent);
	return EXIT_FAILURE;
    }
    
    // listen on inventory messages
    //bios_agent_set_consumer (agent, bios_get_stream_main (), "^inventory@.+"); 
    mlm_client_set_consumer(agent, BIOS_PROTO_STREAM_ASSETS, ".*");

    while ( !zsys_interrupted )
    {
        // ASSUMPTION:
        //  messages will never be lost, so no need to manage a queue
        //  of sended messages
        
        zmsg_t *new_msg = mlm_client_recv(agent);
	if (!new_msg) {
	      break;
	}
	
	if (!is_bios_proto (new_msg)) {
	     log_warning ("not a bios proto message sender == '%s', subject == '%s",
	       mlm_client_sender (agent), mlm_client_subject (agent));
	     continue;
	}
	  
	bios_proto_t *proto = bios_proto_decode (&new_msg);
	if (!proto) {
	    log_critical ("bios_proto_decode () failed");
	    break;
	}
	
	// WIP
        bios_proto_print(proto);
	
/*        
        log_info ("Command is '%s'", mlm_client_command (agent));
	
        // from stream we can recieve only one type of messages
        if ( streq (mlm_client_command (agent), "STREAM DELIVER") )
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
*/        
        // we should destroy the message
        bios_proto_destroy (&proto);
    }

    mlm_client_destroy (&agent);
    log_info ("%s finished.", BIOS_AGENT_NAME_DB_INVENTORY);
    return EXIT_SUCCESS;
}
