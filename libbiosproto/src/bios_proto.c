// implements new BIOS protocols


// metric

#include "bios_proto.h"

#include <stdio.h>
#include <string.h>

int metric_send (
        mlm_client_t *cl,
        char *type,
        char *element_src,
        char *value,
        char *unit,
        int64_t   timestamp,
        char *element_dest
        ) {

    if (!cl || !type || !element_src || !value || !unit) {
        return -1;
    }
    // timestamp is positive, -1 means current timestamp
    if ( timestamp < -1 ) {
        return -2;
    }

    char *subject, *stimestamp;
    asprintf (&subject, "%s@%s", type, element_src);
    asprintf (&stimestamp, "%"PRIi64, timestamp);

    int r = mlm_client_sendx (cl, subject, type, element_src, value, unit, stimestamp, element_dest, NULL);

    zstr_free (&subject);
    zstr_free (&stimestamp);
    return r;
}

int metric_decode (
        zmsg_t **msg_p,
        char **type,
        char **element_src,
        char **value,
        char **unit,
        int64_t   *timestamp,
        char **element_dest
        ) {

    if (!msg_p || !*msg_p || !type || !element_src || !value || !unit || !timestamp) {
        return -1;
    }

    zmsg_t *msg = *msg_p;
    if ( ( zmsg_size(msg) < 5 ) || ( zmsg_size(msg) > 6 ) ) {
        zmsg_destroy (&msg);
        return -2;
    }
    *type = zmsg_popstr (msg);
    *element_src = zmsg_popstr (msg);
    *value = zmsg_popstr (msg);
    *unit = zmsg_popstr (msg);

    char *stme = zmsg_popstr (msg);
    char *endptr;
    errno = 0;
    long int foo = strtol (stme, &endptr, 10);
    if (errno != 0) {
        *timestamp = -1;
    }
    else {
        *timestamp = foo;
    }
    errno = 0;
    zstr_free (&stme);

    if (*timestamp == -1) {
        *timestamp = time (NULL);
    }

    if (element_dest)
    {
        if ( zmsg_size(msg) != 1 ) {
            zmsg_destroy (&msg);
            return -2;
        }
        *element_dest = zmsg_popstr(msg);
    }

    zmsg_destroy (&msg);
    return 0;
}

int alert_send (
        mlm_client_t *cl,
        char *rule_name,
        char *element_name,
        int64_t timestamp,
        char *state,
        char *severity
        )
{
    if (!cl || !rule_name || !element_name || !state || !severity) {
        return -1;
    }
    // timestamp is positive, -1 means current timestamp
    if ( timestamp < -1 ) {
        return -2;
    }

    char *subject, *stimestamp;
    asprintf (&subject, "%s/%s@%s", rule_name, severity, element_name);
    asprintf (&stimestamp, "%"PRIi64, timestamp);

    int r = mlm_client_sendx (cl, subject, rule_name, element_name, stimestamp, state, severity, NULL);

    zstr_free (&subject);
    return r;

}

int alert_decode (
        zmsg_t **msg_p,
        char **rule_name,
        char **element_name,
        int64_t *timestamp,
        char **state,
        char **severity
        )
{
    if (!msg_p || !*msg_p || !rule_name || !element_name || !state || !severity || !timestamp ) {
        return -1;
    }

    zmsg_t *msg = *msg_p;
    if ( zmsg_size(msg) != 5 ) {
        zmsg_destroy (&msg);
        return -2;
    }
    *rule_name = zmsg_popstr (msg);
    *element_name = zmsg_popstr (msg);

    char *stme = zmsg_popstr (msg);
    char *endptr;
    errno = 0;
    long int foo = strtol (stme, &endptr, 10);
    if (errno != 0) {
        *timestamp = -1;
    }
    else {
        *timestamp = foo;
    }
    errno = 0;
    zstr_free (&stme);

    if (*timestamp == -1) {
        *timestamp = time (NULL);
    }

    *state = zmsg_popstr (msg);
    *severity = zmsg_popstr (msg);

    zmsg_destroy (&msg);
    return 0;

}
