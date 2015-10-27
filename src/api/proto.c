// implements new BIOS protocols


// metric

#include <malamute.h>

#include <string.h>

int metric_send (
        mlm_client_t *cl,       // malamute client publish the metric, caller is responsible for correct initialization
        char *type,             // type of metric
        char *element_src,      // source element
        char *value,            // value of metric
        char *units,            // units ('%', 'kWh', '', ...)
        int64_t   time,         // (optional) unix time of measurement, -1 means current system time
        char *element_dest      // (optional) destionation element or NULL
        ) {

    if (!cl || !type || !element_src || !value || !units) {
        return -1;
    }

    char buf[21];
    memset (buf, '\0', 21);
    sprintf (buf, "%"PRIi64, time);
    return mlm_client_sendx (cl, type, element_src, value, units, buf, element_dest, NULL);
}

int metric_encode (
        zmsg_t **msg_p,       // malamute client publish the metric, caller is responsible for correct initialization
        char **type,             // type of metric
        char **element_src,      // source element
        char **value,            // value of metric
        char **units,            // units ('%', 'kWh', '', ...)
        int64_t   *time,         // (optional) unix time of measurement, -1 means current system time
        char **element_dest      // (optional) destionation element or NULL
        ) {

    //TODO return -1??
    zmsg_t *msg;

    *type = zmsg_popstr(msg);
    *element_src = zmsg_popstr(msg);
    *value = zmsg_popstr(msg);
    *units = zmsg_popstr(msg);
    *time = atol(zmsg_popstr(msg));
    *element_dest = zmsg_popstr(msg);

    zmsg_destroy (&msg);
    return 0;
}

int main() {

// there will be the test

}
