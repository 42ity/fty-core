#include "alert-agent.h"
#include "log.h"
#include "utils_ymsg.h"

#include <errno.h>
#include <iostream>

const char *
app_args_get_string(app_t* msg, const char *key) {
    static const char *nullchar = "";
    
    const char *val = (const char *)zhash_lookup(app_args(msg), key);
    if(val == NULL) {
        errno = EKEYREJECTED;
        return nullchar;
    }
    return val;
}

void
app_args_set_string(app_t* msg, const char *key, const char *value) {
    zhash_t *hash = app_get_args(msg);
    if(hash == NULL) {
        hash = zhash_new();
        zhash_autofree(hash);
    }
    zhash_update(hash, key, (void *)value);
    app_set_args(msg, &hash);
}

int32_t
app_args_get_int32(app_t* msg, const char *key) {
    int32_t ret;
    const char *val = (const char *)zhash_lookup( app_args(msg), key);
    if(val == NULL) {
        errno = EKEYREJECTED;
        return 0;
    }
    if(sscanf(val, "%" SCNi32, &ret) != 1) {
        errno = EBADMSG;
        return 0;
    }
    return ret;
}

int64_t
app_args_get_int64(app_t* msg, const char *key) {
    int64_t ret;
    const char *val = (const char *)zhash_lookup(app_args(msg), key);
    if(val == NULL) {
        errno = EKEYREJECTED;
        return 0;
    }
    if(sscanf(val, "%" SCNi64, &ret) != 1) {
        errno = EBADMSG;
        return 0;
    }
    return ret;
}

void
app_args_set_int64(app_t* msg, const char *key, int64_t value ) {
    char buff[24];
    memset( buff, sizeof(buff), 0 );
    snprintf( buff, sizeof(buff)-1, "%" PRIi64, value );
    app_args_set_string( msg, key, buff );
}

void
app_args_set_int32(app_t* msg, const char *key, int32_t value ) {
    app_args_set_int64( msg, key, value );
}

ymsg_t *
bios_alert_encode (const char *rule_name,
                   alert_priority_t priority,
                   alert_state_t state,
                   const char *devices,
                   const char *alert_description,
                   time_t since)
{
    if(
        ! rule_name || ! devices ||
        ( state < ALERT_STATE_NO_ALERT ) ||
        ( state > ALERT_STATE_ONGOING_ALERT ) ||
        ( priority < ALERT_PRIORITY_P1 ) ||
        ( priority > ALERT_PRIORITY_P5 )
    ) return NULL;
    ymsg_t *msg = ymsg_new(YMSG_SEND);
    app_t *app = app_new(APP_MODULE);
    app_set_name( app, "ALERT" );
    app_args_set_string( app, "rule", rule_name );
    app_args_set_int32( app, "priority", priority );
    app_args_set_int32( app, "state", state );
    app_args_set_string( app, "devices",  devices );
    if(alert_description) app_args_set_string(app, "description",  alert_description );
    app_args_set_int64( app, "since", since );
    ymsg_request_set_app( msg, &app );
    return msg;
}


int bios_alert_decode (ymsg_t *self_p,
                       char **alert_name,
                       alert_priority_t *priority,
                       alert_state_t *state,
                       char **devices,
                       char **description,
                       time_t *since)
{
   if( ! self_p || ! alert_name || ! priority || ! state || ! devices ) return -1;
   if ( self_p == NULL ) return -2;

   const char *nam, *dev, *pri, *sta, *sin, *des;
   int32_t tmp;

   app_t *app = ymsg_request_app(self_p);
   if( ! app ) return -2;
       
   nam = app_args_get_string( app, "alert" );
   pri = app_args_get_string( app, "priority" );
   sta = app_args_get_string( app, "state" );
   dev = app_args_get_string( app, "devices" );
   des = app_args_get_string( app, "description" );
   sin = app_args_get_string( app, "since" );
   
   if( ! nam || ! pri || ! sta || ! dev || ! sin ) return -3;

   tmp = app_args_get_int32( app, "priority" );
   if( tmp < ALERT_PRIORITY_P1 || tmp > ALERT_PRIORITY_P5 ) return -4;
   *priority = (alert_priority_t)tmp;
   tmp = app_args_get_int32( app, "state" );
   if( tmp < ALERT_STATE_NO_ALERT || tmp > ALERT_STATE_ONGOING_ALERT ) return -5;
   *state = (alert_state_t)tmp;
   
   app_destroy(&app);
   
   *alert_name = strdup(nam);
   *devices = strdup(dev);
   if( description ) {
       if( des ) {
           *description = strdup(des);
       } else {
           *description = NULL;
       }
   }
   if( since ) {
       *since = ymsg_get_int32( self_p, "since" );
   }
   return 0;
}


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
