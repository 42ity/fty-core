#include "alerts.h"
#include "app.h"
#include "log.h"
#include "dbpath.h"
#include "bios_agent.h"

#include<string.h>

namespace persist {

//! Process alert message and creates an answer
void process_alert(ymsg_t* out, char** out_subj,
                   ymsg_t* in, const char* in_subj)
{
    if( ! in || ! *in ) return;
    
    *out_subj = NULL;
    if( in_subj ) {
        *out_subj = strdup(in_subj);
    }
    log_debug("processing alert"); // FIXME: some macro
    ymsg_t *copy = ymsg_dup(in);
    assert(copy);

    char *rule = NULL, *devices = NULL;
    alert_priority_t priority;
    alert_state_t state;
    time_t since;
    if( bios_alert_decode( &copy, &rule, &priority, &state, &devices, NULL, &since) != 0 ) {
        ymsg_destroy(&copy);
        return
    }
    // decode message
      //open db
    tntdb::Connection conn;
    try{
        
        conn = tntdb::connect(url);
        if( state == ALERT_STATE_ONGOING_ALERT ) {
            // alert started
            auto ret = insert_new_alert(
                conn,
                rule,
                priority,
                state,
                rule, // FIXME: desc
                0,
                time_t);
            //FIXME ret.status
        }                
        else {
            //alarm end
            auto ret = update_alert_tilldate_by_rulename(
                conn,
                since,
                rule);
        }
        catch(...) {
            // put failed into ymsg
        }
    }
    if(rule) free(rule);
    if(devices) free(rule);
    log_debug("processing alert end"); // FIXME: some macro
}

/*
  void process_get_asset(ymsg_t* out, char** out_subj,
                       ymsg_t* in, const char* in_subj)
{
    *out_subj = NULL;
    if( in_subj ) {
        *out_subj = strdup(in_subj);
    }
    app_t *app = app_new(APP_MODULE);
    app_set_name('ASSET');
    ymsg_response_set_app( out, app );
}
*/

} //namespace persist
