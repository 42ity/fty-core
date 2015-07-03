#include "cleanup.h"
#include "sample_agent.h"
#include "defs.h"
#include "log.h"

#include <stdio.h>
#include <zsys.h>
#include <map>
#include <unistd.h>

// Get configuration
extern sample_agent agent;

int main (int argc, char *argv []) {
    // Basic settings
    if (argc > 1) {
        printf ("syntax: %s [ ipc://...|tcp://... ]\n", argv[0]);
        return 0;
    }

    const char *addr = (argc == 1) ? "ipc://@/malamute" : argv[1];
    std::map<std::string, std::pair<int, time_t>> cache;

    if(agent.init != NULL && agent.init())
        return -1;

    // Form ID from hostname and agent name
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);
    std::string id = std::string(agent.agent_name) + "@" + hostname;

    // Create client
    _scoped_bios_agent_t *client = bios_agent_new(addr, id.c_str());
    if(!client) {
        log_error("server not reachable at %s", addr);
        return 0;
    }
    log_info("Connected to %s", addr);
    bios_agent_set_producer(client, bios_get_stream_main());
    log_info("Publishing to %s as %s", bios_get_stream_main(), id.c_str());

    // Until interrupted
    while(!zsys_interrupted) {
        // Go through all the stuff we monitor
        char **what = agent.variants;
        while(what != NULL && *what != NULL && !zsys_interrupted) {
            // Get measurement
            ymsg_t* msg = agent.get_measurement(*what);
            if(msg == NULL) {
                zclock_sleep (100);
                what++;
                continue;
            }
            // Check cache to see if updated value needs to be send
            auto cit = cache.find(*what);
            if((cit == cache.end()) ||
               (abs(cit->second.first - ymsg_get_int32(msg, "value")) > agent.diff) ||
               (time(NULL) - cit->second.second > AGENT_NUT_REPEAT_INTERVAL_SEC)) {
                if(cit == cache.end())
                    cache.insert(std::make_pair(*what,
                        std::make_pair(ymsg_get_int32(msg, "value"), time(NULL))));
                // Prepare topic from templates
                char* topic = (char*)malloc(strlen(agent.at) +
                                            strlen(agent.measurement) + 
                                            strlen(hostname) +
                                            strlen(*what) + 5);
                sprintf(topic, agent.at, hostname);
                ymsg_set_string(msg, "device", topic);
                sprintf(topic, agent.measurement, *what);
                ymsg_set_string(msg, "quantity", topic);
                strcat(topic, "@");
                sprintf(topic + strlen(topic), agent.at, hostname);
                log_info("Sending new measurement '%s' with value %" PRIi32 " * 10^%" PRIi32,
                         topic, ymsg_get_int32(msg, "value"),
                                ymsg_get_int32(msg, "scale"));
                // Send it
                bios_agent_send(client, topic, &msg);
                zclock_sleep (100);
            } else {
                ymsg_destroy(&msg);
            }
            what++;
        }
        // Hardcoded monitoring interval
        zclock_sleep(NUT_POLLING_INTERVAL);
    }

    bios_agent_destroy(&client);
    if(agent.close != NULL && agent.close())
        return -1;
    return 0;
}
