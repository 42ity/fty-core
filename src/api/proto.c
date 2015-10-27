// implements new BIOS protocols


// metric
#include <malamute.h>

#include <stdio.h>
#include <string.h>

int metric_send (
        mlm_client_t *cl,       // malamute client publish the metric, caller is responsible for correct initialization
        char *type,             // type of metric
        char *element_src,      // source element
        char *value,            // value of metric
        char *unit,             // unit ('%', 'kWh', '', ...)
        int64_t   time,         // (optional) unix time of measurement, -1 means current system time
        char *element_dest      // (optional) destionation element or NULL
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
        zmsg_t **msg_p,       // malamute client publish the metric, caller is responsible for correct initialization
        char **type,             // type of metric
        char **element_src,      // source element
        char **value,            // value of metric
        char **unit,            // unit ('%', 'kWh', '', ...)
        int64_t   *tme,         // (optional) unix time of measurement, -1 means current system time
        char **element_dest      // (optional) destionation element or NULL
        ) {

    if (!msg_p || !*msg_p || !type || !element_src || !value || !unit || !tme) {
        return -1;
    }

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

    if (element_dest)
        *element_dest = zmsg_popstr(msg);

    zmsg_destroy (&msg);
    return 0;
}

int main() {

    static const char *endpoint = "inproc://@/malamute";

    zactor_t *server = zactor_new (mlm_server, "Malamute");
    zstr_sendx (server, "BIND", endpoint, NULL);

    mlm_client_t *producer = mlm_client_new();
    mlm_client_connect (producer, endpoint, 5000, "producer");
    mlm_client_set_producer (producer, "ALERTS");

    mlm_client_t *consumer = mlm_client_new();
    mlm_client_connect (consumer, endpoint, 5000, "consumer");
    mlm_client_set_consumer (consumer, "ALERTS", ".*");

    // send
    int r = metric_send (producer, "TYPE", "ELEMENT_SRC", "VALUE", "UNITS", -1, "ELEMENT_DEST");
    assert (r == 0);

    // recv
    zmsg_t *msg = mlm_client_recv (consumer);
    assert (msg);

    char *type, *element_src, *value, *unit, *element_dest;
    int64_t tme;
    r = metric_decode (&msg, &type, &element_src, &value, &unit, &tme, &element_dest);
    assert (r == 0);

    assert (streq (type, "TYPE"));
    assert (streq (element_dest, "ELEMENT_DEST"));

    zstr_free (&type);
    zstr_free (&element_src);
    zstr_free (&value);
    zstr_free (&unit);
    zstr_free (&element_dest);

    // send
    r = metric_send (producer, "TYPE", "ELEMENT_SRC", "VALUE", "UNITS", 42, NULL);
    assert (r == 0);

    //recv
    msg = mlm_client_recv (consumer);
    assert (msg);
    r = metric_decode (&msg, &type, &element_src, &value, &unit, &tme, &element_dest);
    assert (r == 0);

    assert (tme == 42);
    assert (element_dest == NULL);

    mlm_client_destroy (&producer);
    mlm_client_destroy (&consumer);
    zactor_destroy (&server);

}
