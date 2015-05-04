#include <string>

#include "assetcrud.h"

#include "log.h"
#include "defs.h"
#include "agents.h"
#include "dbpath.h"
#include "bios_agent.h"
#include "utils.h"
#include "utils_ymsg.h"
#include "utils_app.h"

namespace persist {

void process_get_asset(ymsg_t* out, char** out_subj,
                       ymsg_t* in, const char* in_subj)
{
    if( ! in || ! out ) return;

    *out_subj = strdup( in_subj );
    ymsg_set_status( out, false );
    
    tntdb::Connection conn;
    try{        
        conn = tntdb::connect(url);
        char *devname = NULL;
        if( bios_asset_extract( in, &devname, NULL, NULL, NULL, NULL ) == 0 ) {
            // db_reply <db_a_elmnt_t> element = select_asset_element_by_name(conn, devname);
            auto element = select_asset_element_by_name(conn, devname);
            if( element.status ) {
                /*
                ymsg_t *tmp = bios_asset_encode(
                    element.item.name.c_str(),
                    element.item.type_id,
                    element.item.parent_id,
                    element.item.status.c_str(),
                    element.item.priority);
                if( tmp ) {
                    zchunk_t *app =  ymsg_get_request(tmp);
                    ymsg_set_response( out, &app );
                }
                ymsg_set_status( out, tmp != NULL );
                ymsg_destroy( &tmp );
                */
                app_t *app = app_new(APP_MODULE);
                if( app ) {
                    log_debug("Setting ASSET reply for %s\n", element.item.name.c_str() );
                    app_set_name( app, "ASSET" );
                    app_args_set_string( app, "devicename", element.item.name.c_str() );
                    app_args_set_uint32( app, "type_id", element.item.type_id );
                    app_args_set_uint32( app, "parent_id", element.item.parent_id );
                    app_args_set_string( app, "status", element.item.status.c_str() );
                    app_args_set_int32( app, "priority", element.item.priority );
                    ymsg_response_set_app( out, &app );
                    ymsg_set_status( out, true );
                    app_destroy( &app );
                }
            } else {
                log_error("Setting ASSET reply for %s failed\n", devname );
            }
        }
        FREE0(devname);
    } catch(const std::exception &e) {
        LOG_END_ABNORMAL(e);
        ymsg_set_status( out, false );
    }
}

} // namespace
