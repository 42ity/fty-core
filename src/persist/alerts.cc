#include "alerts.h"
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


} //namespace persist
