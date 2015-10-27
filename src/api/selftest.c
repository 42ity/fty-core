#include "proto.h"

int main() {

    static const char *endpoint = "inproc://@/malamute";

    zactor_t *server = zactor_new (mlm_server, "Malamute");
    zstr_sendx (server, "BIND", endpoint, NULL);

    mlm_client_t *producer = mlm_client_new();
    mlm_client_connect (producer, endpoint, 5000, "producer");
    mlm_client_set_producer (producer, "METRICS");

    mlm_client_t *consumer = mlm_client_new();
    mlm_client_connect (consumer, endpoint, 5000, "consumer");
    mlm_client_set_consumer (consumer, "METRICS", ".*");

    // Test case: send all values
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
    assert (tme != -1);
    assert (tme - time(NULL) < 15);

    zstr_free (&type);
    zstr_free (&element_src);
    zstr_free (&value);
    zstr_free (&unit);
    zstr_free (&element_dest);

    // Test case: have element_dest NULL
    // send
    r = metric_send (producer, "TYPE", "ELEMENT_SRC", "VALUE", "UNITS", 42, NULL);
    assert (r == 0);

    //recv
    msg = mlm_client_recv (consumer);
    assert (msg);
    r = metric_decode (&msg, &type, &element_src, &value, &unit, &tme, NULL);
    assert (r == 0);

    assert (tme == 42);
    assert (element_dest == NULL);

    zstr_free (&type);
    zstr_free (&element_src);
    zstr_free (&value);
    zstr_free (&unit);
    zstr_free (&element_dest);

    mlm_client_destroy (&producer);
    mlm_client_destroy (&consumer);
    zactor_destroy (&server);
}
