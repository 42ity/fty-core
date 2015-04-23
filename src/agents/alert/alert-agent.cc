#include "alert-agent.h"
#include "agents.h"
#include "log.h"
#include "utils_ymsg.h"
#include "utils-app.h"

#include <errno.h>
#include <iostream>


#define ALERT_POLLING_INTERVAL  5000

void AlertAgent::onStart( )
{
    _timeout = ALERT_POLLING_INTERVAL;
}

void AlertAgent::onSend( ymsg_t **message )
{
    // hope we get some measurement
    std::cout << "onSend\n";
    _model.newMeasurement( *message );
    ymsg_destroy( message );
    std::cout << "onSend end\n";
}

void AlertAgent::onReply( ymsg_t **message )
{
    if( ! message || ! *message ) return;
    std::cout << "onReply\n";
    app_t *app = ymsg_request_app( *message );
    if(app) {
        // is this an alarm confirmation ?
        const char *name = app_name(app);
        if( name && strcmp(name, "ALERT") == 0 ) {
            // name is alert, let's check rule and state 
            std::cout << "ALERT CONFIRMATION...\n";
            app_print(app);
            zhash_t *args = app_args(app);
            char *rule = (char *)zhash_lookup(args, "rule");
            char *charstate =  (char *)zhash_lookup(args, "state");
            if( rule && charstate ) {
                // rule and state supplied
                alert_state_t state = (alert_state_t)atoi(charstate);
                Alert *A = _model.alertByRule(rule);
                if( A && ( A->state() == state ) ) {
                    // such alarm exists and it is in the same state
                    std::cout << "informed\n";
                    A->persistenceInformed(true);
                }
            }
            //zhash_destroy(&args);
        } // name == "ALERT"
        app_destroy(&app);
    } // if(app)
    ymsg_destroy( message );
    std::cout << "onReply end\n";
}

void AlertAgent::onPoll() {
    std::cout << "onPoll\n";
    for( auto &al : _model.alerts() ) {
        if( al.timeToPublish() ) {
            ymsg_t * msg = bios_alert_encode (
                al.ruleName().c_str(),
                al.priority(),
                al.state(),
                al.devices().c_str(),
                NULL, // TODO get description into alert
                al.since());
            std::string topic = "alert." + al.name() + "@" + al.devices();
            if( ! al.persistenceInformed() ) {
                ymsg_t *pmsg = ymsg_dup(msg);
                if( pmsg ) {
                    ymsg_set_repeat( pmsg, true );
                    sendto("persistence.measurement",topic.c_str(),&pmsg);
                }
                ymsg_destroy(&pmsg);
            }
            send( topic.c_str(), &msg );
            ymsg_destroy(&msg);
            al.published();
            zclock_sleep(100);
        }
    }
    std::cout << "onPoll end\n";
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
    if( agent.connect("ipc://@/malamute", bios_get_stream_main(), "^measurement\\..+") ) {
        result = agent.run();
    }
    log_info ("alert agent exited with code %i\n", result);
    return result;
}
