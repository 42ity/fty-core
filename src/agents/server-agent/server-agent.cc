#include "sample_agent.h"

#include <cxxtools/directory.h>
#include <cxxtools/fileinfo.h>

#include <vector>
#include <string>

#define THERMAL "/sys/class/thermal"

int agent_init();
int agent_close();
ymsg_t* get_measurement(char* what);

sample_agent agent = {
    "server-agent",
    agent_init,
    agent_close,
    NULL,
    "temperature.sys.%s",
    "%s",
    1,
    get_measurement
};

int agent_init() {
    std::vector<std::string> thermals;
    cxxtools::Directory d(THERMAL);
    for(cxxtools::Directory::const_iterator it = d.begin(); it != d.end(); ++it) {
        FILE* f = fopen((std::string(THERMAL) + "/" + *it + "/temp").c_str(), "r");
        // Try temp file in there
        if(f != NULL) {
            thermals.push_back(*it);
            fclose(f);
        }
    }
    agent.variants = (char**)malloc(sizeof(char*)*(thermals.size()+1));
    int i = 0;
    for(auto t : thermals) {
        agent.variants[i++] = strdup(t.c_str());
    }
    agent.variants[i++] = NULL;
    return 0;
}

int agent_close() {
    int i = 0;
    while(agent.variants[i] != NULL) {
        free(agent.variants[i]);
        agent.variants[i++] = NULL;
    }
    free(agent.variants);
    agent.variants = NULL;
    return 0;
}

ymsg_t* get_measurement(char* what) {
    int temp;
    ymsg_t* ret = NULL;
    std::string path = std::string(THERMAL) + "/" + what + "/temp";
    FILE* f = fopen(path.c_str(), "r");
    if(f != NULL) {
        if(fscanf(f, "%d", &temp) == 1) {
            ret = bios_measurement_encode("", "", "C", temp / 100, -1, -1);
        }
        fclose(f);
    }
    return ret;
}
