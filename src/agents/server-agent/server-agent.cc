/*
 *
 * Copyright (C) 2015 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*!
 * \file server-agent.cc
 * \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 * \brief Not yet documented file
 */
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
    10,
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
