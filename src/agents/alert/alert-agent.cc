#include "alert-agent.h"

#include <errno.h>
#include <iostream>

#include "agents.h"
#include "log.h"
#include "utils.h"
#include "utils_ymsg.h"
#include "utils_app.h"
#include "str_defs.h"

#define ALERT_POLLING_INTERVAL  5000
#define check_ymsg(X) ( X && *X )

void AlertAgent::onStart( )
{
    _timeout = ALERT_POLLING_INTERVAL;
}

void AlertAgent::onSend( ymsg_t **message )
{
    // hope we get some measurement
    if( ! check_ymsg( message ) ) return;
    _model.newMeasurement( *message );
    ymsg_destroy( message );
}

void AlertAgent::onReply( ymsg_t **message )
{
    if( ! check_ymsg(message) && ! ymsg_is_ok(*message) ) return;
    if( ! streq( sender(), BIOS_AGENT_NAME_DB_MEASUREMENT ) ) return;

    //    app_t *app = ymsg_request_app( *message );
    const char *topic = subject();
    if( topic && strncmp( topic, "alert.", 6 ) == 0 ) {
        // this should be alert confirmation from persistence
        char *rule = NULL, *devices = NULL;
        int8_t state = 0;
        uint8_t priority = 0;
        if( bios_alert_extract( *message, &rule, &priority, &state, &devices, NULL, NULL ) == 0 ) {
            auto it = _model.alertByRule(rule);
            if( ( it != _model.end() ) && ( it->second.state() == state ) ) {
                // such alarm exists and it is in the same state
                it->second.persistenceInformed(true);
            }
        }
        FREE0(rule);
        FREE0(devices);
    }
    else if( streq( topic, "get_asset_element" ) ) {
        char *device = NULL;
        uint8_t priority;
        if( bios_asset_extract( *message, &device, NULL, NULL, NULL, &priority) == 0 ) {
            log_debug( "Device %s priority is %i", device, priority );
            _model.setPriority( device, (alert_priority_t)priority );
        }
        FREE0( device );
    }
}

void AlertAgent::onPoll() {
    for( auto al = _model.begin(); al != _model.end() ; ++al ) {
        if( al->second.priority() == ALERT_PRIORITY_UNKNOWN ) {
            // hmm, ask for priority
            ymsg_t * msg = bios_asset_encode( al->second.devices().c_str(), 0, 0, NULL, 0);
            if( msg ) {
                ymsg_set_repeat( msg, true ); // FIXME: failed if not used
                log_debug( "Sending request for device %s priority", al->second.devices().c_str() );
                sendto( BIOS_AGENT_NAME_DB_MEASUREMENT, "get_asset_element", &msg );
                ymsg_destroy(&msg);
            }
        }
        else
        if( al->second.timeToPublish() ) {
            ymsg_t * msg = bios_alert_encode (
                al->second.ruleName().c_str(),
                al->second.priority(),
                al->second.state(),
                al->second.devices().c_str(),
                al->second.description().c_str(),
                al->second.since());
            std::string topic = "alert." + al->second.ruleName();
            if( ! al->second.persistenceInformed() ) {
                ymsg_t *pmsg = ymsg_dup(msg);
                if( pmsg ) {
                    ymsg_set_repeat( pmsg, true );
                    log_debug("sending alert %s state %i to persistence", al->second.ruleName().c_str(), al->second.state() );
                    sendto( BIOS_AGENT_NAME_DB_MEASUREMENT, topic.c_str(), &pmsg );
                }
                ymsg_destroy(&pmsg);
            }
            log_debug("Advertising alert %s state %i", al->second.ruleName().c_str(), al->second.state() );
            send( topic.c_str(), &msg );
            ymsg_destroy(&msg);
            al->second.published();
            zclock_sleep(100);
        }
    }
}


int main( int argc, char *argv[] )
{
    if( argc > 0 ) {}; // silence compiler warnings
    if( argv ) {};  // silence compiler warnings
    
    int result = 1;
    log_open();
    log_set_level(LOG_DEBUG);
    log_info ("alert agent started");
    AlertAgent agent("ALERTS");
    if( agent.connect(MLM_ENDPOINT, bios_get_stream_main(), "^measurement\\..+") ) {
        result = agent.run();
    }
    log_info ("alert agent exited with code %i\n", result);
    return result;
}
