#include "alerts.h"
#include "app.h"

#include<string.h>

namespace persist {

//! Process alert message and creates an answer
void process_alert(ymsg_t* out, char** out_subj,
                   ymsg_t* in, const char* in_subj)
{
    *out_subj = NULL;
    if( in_subj ) {
        *out_subj = strdup(in_subj);
    }
    /*
      log_debug()

      // decode message
      //open db
      tntdb::Connection conn;
      try{
          conn = tntdb::connect(url);
          if(ongoing)
            // alert started
            auto ret = insert_new_alert(
        else
            //alarm end
            update_alert_tilldate_by_rulename

        }
                ret.status == 1 => OK
                // put ok do ymsg
                catch(...) {

                    // put failed into ymsg
                }
        */
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
