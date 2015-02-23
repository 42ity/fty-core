#include "malamute.h"
#include "common_msg.h"

#include <cxxtools/directory.h>
#include <cxxtools/fileinfo.h>

#include <stdio.h>
#include <zsys.h>
#include <map>

#define CLIENT_ID "server-agent"
#define THERMAL "/sys/class/thermal"

int main (int argc, char *argv []) {
    int ret = 0;
    common_msg_t *dta = NULL;
    int temp = 0;
    int type_id = 0;

    // Basic settings
    if (argc > 1) {
        printf ("syntax: server-agent [ ipc://...|tcp://... ]\n");
        return 0;
    }
    char id[HOST_NAME_MAX + sizeof(CLIENT_ID)+5];
    const char *addr = (argc == 1) ? "ipc://@/malamute" : argv[1];

    std::map<std::string, int> cache;

    // Form ID from hostname
    strcpy(id, CLIENT_ID ".");
    gethostname(id + sizeof(CLIENT_ID), HOST_NAME_MAX - sizeof(CLIENT_ID));

    // Create client
    mlm_client_t *client = mlm_client_new(addr, 1000, id);
    if (!client) {
        zsys_error ("server-agent: server not reachable at ipc://@/malamute");
        return 0;
    }

    // Register client
    zmsg_t *req = common_msg_encode_insert_client(
                  common_msg_encode_client(id));
    mlm_client_sendto(client, "persistence", "persistence", NULL, 0, &req);
    zmsg_t* rep = mlm_client_recv(client);
    if(rep == NULL) {
        fprintf(stderr, "Receiving reply to inserting client failed!\n");
        goto exit;
    }
    dta = common_msg_decode(&rep);
    if(dta == NULL) {
        fprintf(stderr, "Can't decode reply to inserting client!\n");
        goto exit;
    }
    common_msg_destroy(&dta);

    // Register device
    // 4 is in my case server - TODO: Replace with correct type query
    req = common_msg_encode_insert_device(
                  common_msg_encode_device(4, id + sizeof(CLIENT_ID)));
    mlm_client_sendto(client, "persistence", "persistence", NULL, 0, &req);
    rep = mlm_client_recv(client);
    if(rep == NULL) {
        fprintf(stderr, "Receiving message failed!\n");
        goto exit;
    }
    dta = common_msg_decode(&rep);
    if(dta == NULL) {
        fprintf(stderr, "Received wrong message!\n");
        goto exit;
    }
    common_msg_destroy(&dta);

    // Get numeric type id for temperature
    req = common_msg_encode_get_measure_type_s(
                        "temperature","C");
    mlm_client_sendto(client, "persistence", "persistence", NULL, 0, &req);
    rep = mlm_client_recv(client);
    if(rep == NULL) {
        fprintf(stderr, "Receiving message failed!\n");
        goto exit;
    }
    dta = common_msg_decode(&rep);
    if(dta == NULL) {
        fprintf(stderr, "Received wrong message!\n");
        goto exit;
    }
    type_id = common_msg_mt_id(dta);
    // We measure temperature only in Celsius
    if((common_msg_mt_unit(dta) == NULL) ||
       strneq(common_msg_mt_unit(dta), "C")) {
        fprintf(stderr, "Measuring in '%s' is not supported!\n",
                common_msg_mt_unit(dta));
        goto exit;
    }
    common_msg_destroy(&dta);

    // Until interrupted
    while(!zsys_interrupted) {
        // Go through all subdirectories in /sys/class/thermal
        cxxtools::Directory d(THERMAL);
        for (cxxtools::Directory::const_iterator it = d.begin();
             it != d.end(); ++it) {
            FILE* f = fopen((std::string(THERMAL) + "/" + *it + "/temp").c_str(), "r");
            // Try temp file in there
            if(f != NULL) {
                // And try readin number from it
                if(fscanf(f, "%d", &temp) == 1) {
                    auto cit = cache.find(*it);
                    // Did it changed singificantly since the last time?
                    if((cit == cache.end()) || (abs(cit->second - temp) > 1000)) {
                        if(cit == cache.end()) {
                            cache.insert(std::make_pair(*it, temp));
                        } else {
                            cit->second = temp;
                        }

                        // Get correct subtype for this particular subdirectory
                        zmsg_t *req = common_msg_encode_get_measure_subtype_s(
                                                type_id, it->c_str(), (uint8_t)-3);
                        mlm_client_sendto(client, "persistence", 
                                                "persistence", NULL, 0, &req);
                        zmsg_t *rep = mlm_client_recv(client);
                        if(rep == NULL)
                            break;
                        common_msg_t *dta = common_msg_decode(&rep);
                        if(dta == NULL)
                            continue;
                        if(((int8_t)common_msg_mts_scale(dta)) != -3)
                            continue;

                        // Create message and publish it
                        zmsg_t *msg = common_msg_encode_new_measurement(
                            id, id + sizeof(CLIENT_ID),"server",
                            type_id, common_msg_mts_id(dta), temp);
                        assert(msg);

                        mlm_client_set_producer(client, "measurements");
                        printf("Sending %s = %d\n", (*it).c_str(), temp);
                        mlm_client_send(client, id, &msg);
                        // To make sure message is send before we do something else
                        // https://github.com/Malamute/malamute-core/issues/35
                        zclock_sleep (100);
                    }
                }
                fclose(f);
            }
        }
        // Hardcoded monitoring interval
        sleep(1);
    }

exit:
    mlm_client_destroy (&client);
    return ret;
}
