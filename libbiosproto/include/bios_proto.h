
#pragma once

#include <malamute.h>

#ifdef __cplusplus
extern "C" {
#endif


int metric_send (
        mlm_client_t *cl,       // malamute client publish the metric, caller is responsible for correct initialization
        char *type,             // type of metric
        char *element_src,      // source element
        char *value,            // value of metric
        char *unit,             // unit ('%', 'kWh', '', ...)
        int64_t  timestamp,     // (optional) unix time of measurement, -1 means current system time
        char *element_dest      // (optional) destionation element or NULL
        );

int metric_decode (
        zmsg_t **msg_p,         // message to decode, message is destroyed
        char **type,            // type of metric
        char **element_src,     // source element
        char **value,           // value of metric
        char **unit,            // unit ('%', 'kWh', '', ...)
        int64_t  *timestamp,    // (optional) unix time of measurement, -1 means current system time
        char **element_dest     // (optional) destionation element or NULL
        );

int alert_send (
        mlm_client_t *cl,       // malamute client publish the metric, caller is responsible for correct initialization
        char *rule_name,        // rule name that case alert evaluation
        char *element_name,     // element where alert was evaluated
        int64_t  timestamp,     // unix time when state was changed, -1 means current system time
        char *state,            // state of the alert
        char *severity          // severity of the alert
        );

int alert_decode (
        zmsg_t **msg_p,          // message to decode, message is destroyed
        char **rule_name,        // rule name that case alert evaluation
        char **element_name,     // element where alert was evaluated
        int64_t  *timestamp,     // unix time when state was changed, -1 means current system time
        char **state,            // state of the alert
        char **severity          // severity of the alert
        );

#ifdef __cplusplus
}
#endif

