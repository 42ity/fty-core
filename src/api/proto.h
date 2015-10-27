
#pragma once

#include <malamute.h>

int metric_send (
        mlm_client_t *cl,       // malamute client publish the metric, caller is responsible for correct initialization
        char *type,             // type of metric
        char *element_src,      // source element
        char *value,            // value of metric
        char *unit,             // unit ('%', 'kWh', '', ...)
        int64_t   time,         // (optional) unix time of measurement, -1 means current system time
        char *element_dest      // (optional) destionation element or NULL
        );

int metric_decode (
        zmsg_t **msg_p,       // malamute client publish the metric, caller is responsible for correct initialization
        char **type,             // type of metric
        char **element_src,      // source element
        char **value,            // value of metric
        char **unit,            // unit ('%', 'kWh', '', ...)
        int64_t   *tme,         // (optional) unix time of measurement, -1 means current system time
        char **element_dest      // (optional) destionation element or NULL
        );
