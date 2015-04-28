#include "alert-agent.h"

#include <errno.h>
#include <iostream>

#include "agents.h"
#include "log.h"
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

    app_t *app = ymsg_request_app( *message );
    if( app ) {
        // is this an alert confirmation?
        const char *name = app_name(app);
        const char *from = sender();
        if( name && strcmp(name, "ALERT") == 0 && from && strcmp(from, "persistence.measurement") == 0 ) {
            // name is alert, and was written into db
            // let's check rule and state 
            const char *rule = app_args_string(app,"rule", NULL);
            int32_t state = app_args_int32(app,"state");
            if( rule && ( state != INT32_MAX ) ) {
                // rule and state supplied
                auto it = _model.alertByRule(rule);
                if( ( it != _model.end() ) && ( it->second.state() == state ) ) {
                    // such alarm exists and it is in the same state
                    it->second.persistenceInformed(true);
                }
            }
        }
        app_destroy(&app);
    }
    ymsg_destroy( message );
}

void AlertAgent::onPoll() {
    for( auto al = _model.begin(); al != _model.end() ; ++al ) {
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
                    log_debug("sending alert %s state %i to persistence\n", al->second.ruleName().c_str(), al->second.state() );
                    sendto("persistence.measurement",topic.c_str(),&pmsg); // FIXME: use str_defs.c, not available now
                }
                ymsg_destroy(&pmsg);
            }
            log_debug("advertising alert %s state %i\n", al->second.ruleName().c_str(), al->second.state() );
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
