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

/*! \file generate_measurement.cc
 *  \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 *  \brief Not yet documented file
 */
#include "cleanup.h"
#include "bios_agent.h"
#include "defs.h"

#include <stdio.h>
#include <zsys.h>
#include <map>

#include "agents.h"
#include "defs.h"

int main(int argc, char *argv []) {
    // Basic settings
    if((argc != 4) && (argc != 5)) {
        printf ("syntax: generate-measurement hostname source topic units value [ ipc://...|tcp://... ]\n");
        return 0;
    }

    const char *addr = (argc == 5) ? "ipc://@/malamute" : argv[5];

    // Form ID from pid
    char id[128];
    snprintf(id, 128, "fake_agent.%d", getpid());

    // Create client
    bios_agent_t *client = bios_agent_new(addr, id);
    if(!client) {
        zsys_error ("server-agent: server not reachable at ipc://@/malamute");
        return 1;
    }

    // Produce meassurement
    bios_agent_set_producer(client, bios_get_stream_main());
    ymsg_t *msg = bios_measurement_encode(argv[1], argv[2], argv[3], atoi(argv[4])*100, -2, -1);
    char *topic = (char*)malloc((strlen(argv[1]) + strlen(argv[2]) + 15) * sizeof(char));
    sprintf(topic, "measurement.%s@%s", argv[2], argv[1]);
    bios_agent_send(client, topic, &msg);
    // To make sure message is send before we do something else
    // https://github.com/Malamute/malamute-core/issues/35
    zclock_sleep (100);

    bios_agent_destroy (&client);
    return 0;
}
