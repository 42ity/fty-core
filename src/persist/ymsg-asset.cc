#include <string>

#include "assetcrud.h"
#include "alert.h"

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

namespace persist {

void process_get_asset(ymsg_t* out, char** out_subj,
                       ymsg_t* in, const char* in_subj)
{
    if( ! in || ! out ) return;
    LOG_START;
    *out_subj = strdup( in_subj );
    ymsg_set_status( out, false );
    
    tntdb::Connection conn;
    try{
        conn = tntdb::connectCached(url);
        char *devname = NULL;
        if( bios_asset_extract( in, &devname, NULL, NULL, NULL, NULL ) == 0 ) {
            auto element = select_asset_element_by_name(conn, devname);
            if( element.status ) {
                app_t *app = app_new(APP_MODULE);
                if( app ) {
                    log_debug("Setting ASSET reply for %s", element.item.name.c_str() );
                    app_set_name( app, "ASSET" );
                    app_args_set_string( app, "devicename", element.item.name.c_str() );
                    app_args_set_uint16( app, "type_id", element.item.type_id );
                    app_args_set_uint32( app, "parent_id", element.item.parent_id );
                    app_args_set_string( app, "status", element.item.status.c_str() );
                    app_args_set_uint8( app, "priority", element.item.priority );
                    ymsg_response_set_app( out, &app );
                    ymsg_set_status( out, true );
                    app_destroy( &app );
                }
            } else {
                log_error("Setting ASSET reply for %s failed", devname );
            }
        }
        FREE0(devname);
    } catch(const std::exception &e) {
        LOG_END_ABNORMAL(e);
        ymsg_set_status( out, false );
    }
    LOG_END;
}

void process_alert(ymsg_t* out, char** out_subj,
                   ymsg_t* in, const char* in_subj)
{
    if( ! in || ! out ) return;

    LOG_START;
    
    if( in_subj ) { *out_subj = strdup(in_subj); }
    else { *out_subj = NULL; }
    
    log_debug("processing alert"); // FIXME: some macro
    
    // decode message
    _scoped_char *rule = NULL, *devices = NULL, *desc = NULL;
    uint8_t priority;
    int8_t state;
    time_t since;
    if( bios_alert_extract( in, &rule, &priority, &state, &devices, &desc, &since) != 0 ) {
        log_debug("can't decode message");
        LOG_END;
        return;
    }
    std::vector<std::string> devices_v;
    cxxtools::split(',', std::string(devices), std::back_inserter(devices_v));
    tntdb::Connection conn;
    try{        
        conn = tntdb::connectCached(url);
        db_reply_t ret;
        
        switch( (int)state ) {
        case ALERT_STATE_ONGOING_ALERT:
            // alert started
            ret = insert_new_alert(
                conn,
                rule,
                priority,
                state,
                ( desc ? desc : rule ),
                0,
                since,
                devices_v);
            ymsg_set_status( out, ret.status );
            break;
        case ALERT_STATE_NO_ALERT:
            //alarm end
            ret = update_alert_tilldate_by_rulename(
                conn,
                since,
                rule);
            ymsg_set_status( out, ret.status );
            break;
        }
        if(!ret.status) { log_error("Writting alert into the database failed"); }
    } catch(const std::exception &e) {
        LOG_END_ABNORMAL(e);
        ymsg_set_status( out, false );
    }
    FREE0 (rule)
    FREE0 (devices)
    LOG_END;
}

} // namespace
