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
