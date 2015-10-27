// implements new BIOS protocols


// metric

#include "proto.h"

#include <stdio.h>
#include <string.h>

int metric_send (
        mlm_client_t *cl,
        char *type,
        char *element_src,
        char *value,
        char *unit,
        int64_t   time,
        char *element_dest
        ) {

    if (!cl || !type || !element_src || !value || !unit) {
        return -1;
    }

    char *subject, *stime;
    asprintf (&subject, "%s@%s", type, element_src);
    asprintf (&stime, "%"PRIi64, time);

    int r = mlm_client_sendx (cl, subject, type, element_src, value, unit, stime, element_dest, NULL);

    zstr_free (&subject);
    zstr_free (&stime);
    return r;
}

int metric_decode (
        zmsg_t **msg_p,
        char **type,
        char **element_src,
        char **value,
        char **unit,
        int64_t   *tme,
        char **element_dest
        ) {

    if (!msg_p || !*msg_p || !type || !element_src || !value || !unit || !tme)
        return -1;

    zmsg_t *msg = *msg_p;

    *type = zmsg_popstr (msg);
    *element_src = zmsg_popstr (msg);
    *value = zmsg_popstr (msg);
    *unit = zmsg_popstr (msg);

    char *stme = zmsg_popstr (msg);
    char *endptr;
    errno = 0;
    long int foo = strtol (stme, &endptr, 10);
    if (errno != 0)
        *tme = -1;
    else
        *tme = foo;
    errno = 0;
    zstr_free (&stme);

    if (*tme == -1)
        *tme = time (NULL);

    if (element_dest)
        *element_dest = zmsg_popstr(msg);

    zmsg_destroy (&msg);
    return 0;
}

int alert_send (
        mlm_client_t *cl,
        char *rule_name,
        char *element_name,
        char *state,
        char *severity
        )
{
    if (!cl || !rule_name || !element_name || !state || !severity) {
        return -1;
    }

    char *subject;
    asprintf (&subject, "%s/%s@%s", rule_name, severity, element_name);

    int r = mlm_client_sendx (cl, subject, rule_name, element_name, state, severity, NULL);

    zstr_free (&subject);
    return r;

}

int alert_decode (
        zmsg_t **msg_p,
        char **rule_name,
        char **element_name,
        char **state,
        char **severity
        )
{
    if (!msg_p || !*msg_p || !rule_name || !element_name || !state || !severity ) {
        return -1;
    }

    zmsg_t *msg = *msg_p;

    *rule_name = zmsg_popstr (msg);
    *element_name = zmsg_popstr (msg);
    *state = zmsg_popstr (msg);
    *severity = zmsg_popstr (msg);

    zmsg_destroy (&msg);
    return 0;

}
