#include "bios_agent.h"

#include <cxxtools/directory.h>
#include <cxxtools/fileinfo.h>

#include <stdio.h>
#include <zsys.h>
#include <map>

#define CLIENT_ID "server-agent"
#define THERMAL "/sys/class/thermal"

int main (int argc, char *argv []) {
    int ret = 0;
    int temp = 0;
    std::string topic;

    // Basic settings
    if (argc > 1) {
        printf ("syntax: server-agent [ ipc://...|tcp://... ]\n");
        return 0;
    }

    const char *addr = (argc == 1) ? "ipc://@/malamute" : argv[1];

    std::map<std::string, int> cache;

    // Form ID from hostname
    char id[HOST_NAME_MAX + sizeof(CLIENT_ID) + 5];
    strcpy(id, CLIENT_ID);
    char *hostname = id + sizeof(CLIENT_ID);
    hostname[0] = '.';
    hostname++;
    gethostname(hostname, HOST_NAME_MAX);

    // Create client
    bios_agent_t *client = bios_agent_new(addr, id);
    if (!client) {
        zsys_error ("server-agent: server not reachable at ipc://@/malamute");
        return 0;
    }

    // Until interrupted
    while(!zsys_interrupted) {
        // Go through all subdirectories in /sys/class/thermal
        cxxtools::Directory d(THERMAL);
        for(cxxtools::Directory::const_iterator it = d.begin();
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

                        // Create message and publish it
                        ymsg_t *msg = ymsg_new(YMSG_SEND);
                        ymsg_set_string(msg, "units", "C");
                        ymsg_set_int32(msg, "value", temp / 100);
                        ymsg_set_int32(msg, "scale", -1);
                        ymsg_set_int64(msg, "time", -1);

                        bios_agent_set_producer(client, "measurements");
                        topic = "temperature.";
                        topic += *it;
                        topic += ".";
                        topic += hostname;
                        printf("Sending %s = %d.%d C\n", topic.c_str(),
                               temp / 1000, (temp % 1000) / 100);
                        bios_agent_send(client, topic.c_str(), &msg);
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
    bios_agent_destroy (&client);
    return ret;
}
