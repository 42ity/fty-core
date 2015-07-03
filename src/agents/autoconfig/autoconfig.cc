/* 
Copyright (C) 2014 - 2015 Eaton
 
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
Author(s): Tomas Halman <tomashalman@eaton.com>

Description: autoconfiguration agent
*/

#include "autoconfig.h"
#include "str_defs.h"
#include "log.h"
#include "utils.h"
#include "asset_types.h"
#include "utils_ymsg.h"

void Autoconfig::onReply( ymsg_t **message )
{
    if( ! message || ! *message ) return;

    ConfigFactory().configureAsset( *message );
}

void Autoconfig::onSend( ymsg_t **message )
{
    if( ! message || ! *message ) return;

    char *device_name = NULL;
    uint32_t type = 0;
    int8_t operation;

    int x;
    if( (x = bios_asset_extract( *message, &device_name, &type, NULL, NULL, NULL, &operation )) == 0 ) {
        log_debug("Asset message for %s type %" PRIu32, device_name, type );
        if( type == asset_type::DEVICE ) {
            ymsg_t *extended = bios_asset_extra_encode( device_name, NULL, 0, 0, NULL, 0, 0, operation );
            if( extended ) {
                log_debug("Sending bios_asset_extra_encode");
                sendto(BIOS_AGENT_NAME_DB_MEASUREMENT,"get_asset_extra", &extended );
                ymsg_destroy( &extended );
            }
        }
    } else {
        log_debug("This is not bios_asset message (error %i)", x);
    }
    FREE0( device_name );
}

int main( int argc, char *argv[] )
{
    if( argc > 0 ) {}; // silence compiler warnings
    if( argv ) {};  // silence compiler warnings
    
    int result = 1;
    log_open();
    log_info ("autoconfig agent started");
    Autoconfig agent("AUTOCONFIG");
    if( agent.connect( MLM_ENDPOINT, bios_get_stream_main(), "^configure@" ) ) {
        result = agent.run();
    }
    log_info ("autoconfig agent exited with code %i\n", result);
    return result;
}
