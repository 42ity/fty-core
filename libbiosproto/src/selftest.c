#include "bios_proto.h"

static const char *endpoint = "inproc://@/malamute";

static void
s_test_metrics (zactor_t *server)
{
    mlm_client_t *producer = mlm_client_new();
    mlm_client_connect (producer, endpoint, 5000, "producer");
    mlm_client_set_producer (producer, "METRICS");

    mlm_client_t *consumer = mlm_client_new();
    mlm_client_connect (consumer, endpoint, 5000, "consumer");
    mlm_client_set_consumer (consumer, "METRICS", ".*");

    // METRICS stream: Test case: send all values
    // send
    int r = metric_send (producer, "TYPE", "ELEMENT_SRC", "VALUE", "UNITS", -1, "ELEMENT_DEST");
    assert (r == 0);

    // recv
    zmsg_t *msg = mlm_client_recv (consumer);
    assert (msg);

    const char* subject = mlm_client_subject (consumer);
    assert (streq (subject, "TYPE@ELEMENT_SRC"));

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

    // METRICS stream: Test case: have element_dest NULL
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
}

static void
s_test_alerts (zactor_t *server)
{
    mlm_client_t *producer = mlm_client_new();
    mlm_client_connect (producer, endpoint, 5000, "producer");
    mlm_client_set_producer (producer, "ALERTS");

    mlm_client_t *consumer = mlm_client_new();
    mlm_client_connect (consumer, endpoint, 5000, "consumer");
    mlm_client_set_consumer (consumer, "ALERTS", ".*");

    // ALERTS stream: Test case: send all values
    // send
    int r = alert_send (producer, "RUNEMANE1", "ELEMENT_NAME", "RESOLVED", "INFO");
    assert (r == 0);

    // recv
    zmsg_t *msg = mlm_client_recv (consumer);
    assert (msg);

    const char* subject = mlm_client_subject (consumer);
    assert (streq (subject, "RUNEMANE1/INFO@ELEMENT_NAME"));

    char *rule_name, *element_name, *status, *severity;
    r = alert_decode (&msg, &rule_name, &element_name, &status, &severity);
    assert (r == 0);

    assert (streq (rule_name, "RULENAME1"));
    assert (streq (element_name, "ELEMENT_NAME"));
    assert (streq (element_name, "RESOLVED"));
    assert (streq (element_name, "INFO"));

    zstr_free (&rule_name);
    zstr_free (&element_name);
    zstr_free (&status);
    zstr_free (&severity);

    mlm_client_destroy (&producer);
    mlm_client_destroy (&consumer);
}

int main() {

    zactor_t *server = zactor_new (mlm_server, "Malamute");
    zstr_sendx (server, "BIND", endpoint, NULL);

    s_test_metrics(server);
    s_test_alerts(server);

    zactor_destroy (&server);
}
