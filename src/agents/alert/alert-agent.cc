#include "alert-agent.h"
#include "log.h"

#include <iostream>

ymsg_t *
bios_alert_encode (const char *alert_name,
                   alert_severity_t severity,
                   alert_state_t state,
                   const char *devices,
                   const char *alert_description,
                   time_t since)
{
    if(
        ! alert_name || ! devices ||
        ( state < ALERT_STATE_NO_ALERT ) ||
        ( state > ALERT_STATE_ONGOING_ALERT ) ||
        ( severity < ALERT_SEVERITY_P1 ) ||
        ( severity > ALERT_SEVERITY_P5 )
    ) return NULL;
    ymsg_t *msg = ymsg_new(YMSG_SEND);
    ymsg_set_string(msg, "alert", (char *)alert_name );
    ymsg_set_int32(msg, "priority", severity );
    ymsg_set_int32(msg, "state", state );
    ymsg_set_string(msg, "devices",  devices );
    if(alert_description) ymsg_set_string(msg, "description",  alert_description );
    ymsg_set_int64(msg, "since", since );
    return msg;
}


int bios_alert_decode (ymsg_t *self_p,
                       char **alert_name,
                       alert_severity_t *severity,
                       alert_state_t *state,
                       char **devices,
                       char **description,
                       time_t *since)
{
   if( ! self_p || ! alert_name || ! severity || ! state || ! devices ) return -1;
   if ( self_p == NULL ) return -2;

   const char *nam, *dev, *sev, *sta, *sin, *des;
   int32_t tmp;
   
   nam = ymsg_get_string( self_p, "alert" );
   sev = ymsg_get_string( self_p, "severity" );
   sta = ymsg_get_string( self_p, "state" );
   dev = ymsg_get_string( self_p, "devices" );
   des = ymsg_get_string( self_p, "description" );
   sin = ymsg_get_string( self_p, "since" );
   
   if( ! nam || ! sev || ! sta || ! dev || ! sin ) return -3;

   tmp = ymsg_get_int32( self_p, "severity" );
   if( tmp < ALERT_SEVERITY_P1 || tmp > ALERT_SEVERITY_P5 ) return -4;
   *severity = (alert_severity_t)tmp;
   tmp = ymsg_get_int32( self_p, "state" );
   if( tmp < ALERT_STATE_NO_ALERT || tmp > ALERT_STATE_ONGOING_ALERT ) return -5;
   *state = (alert_state_t)tmp;
   
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
    
    _model.newMeasurement( *message );
    ymsg_destroy( message );
    /*
    std::cout << "-----------\n";
    _model.print();
    std::cout << "-----------\n";
    */
}

void AlertAgent::onReply( ymsg_t **message )
{
    std::cout << "onReply start\n";
    ymsg_print( *message );
    ymsg_destroy( message );
    std::cout << "onReply end\n";
}

void AlertAgent::onPoll() {
    for( auto &al : _model.alerts() ) {
        if( al.timeToPublish() ) {
            ymsg_t * msg = bios_alert_encode (
                al.name().c_str(),
                al.severity(),
                al.state(),
                al.devices().c_str(),
                NULL, // TODO get description into alert
                al.since());
            std::string topic = "alert." + al.name() + "@" + al.devices();
            if( ! al.persistenceInformed() ) {
            ymsg_print(msg);
                ymsg_t *pmsg = ymsg_dup(msg);
                ymsg_set_repeat( pmsg, true );
                ymsg_print(pmsg);
                sendto("persistence.measurement",topic.c_str(),&pmsg);
                ymsg_destroy(&pmsg);
            }
            // ymsg_print(msg);
            // send( topic.c_str(), &msg );
            ymsg_destroy(&msg);
            al.published();
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
    if( agent.connect("ipc://@/malamute", bios_get_stream_main(), "^measurement\\..+") ) {
        result = agent.run();
    }
    log_info ("alert agent exited with code %i\n", result);
    return result;
}
