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
#include <stdio.h>

#include <czmq.h>
#include <malamute.h>
#include <biosproto.h>

#include "defs.h"

int main(int argc, char *argv []) {
    // Basic settings
    if((argc != 4) && (argc != 5)) {
        printf ("syntax: generate-measurement hostname source units value [ ipc://...|tcp://... ]\n");
        return 0;
    }

    const char *endpoint = (argc == 5) ? "ipc://@/malamute" : argv[5];

    // Form ID from pid
    char address[128];
    snprintf(address, 128, "generate-measurement.%d", getpid());

    // Create client
    mlm_client_t *client = mlm_client_new ();
    assert (client);
    mlm_client_connect (client, endpoint, 5000, address);
    if (!mlm_client_connected (client)) {
        zsys_error ("generate_measurement: mlm server not reachable at %s", endpoint);
        return 1;
    }

    char *type = argv[2];
    char *element_src = argv[1];
    char *value = argv[4];
    char *unit = argv[3];

    // Produce measurement
    mlm_client_set_producer(client, "METRICS");
    zmsg_t *msg = bios_proto_encode_metric(NULL, type, element_src, value, unit, time(NULL));
    assert (msg);

    char *topic;
    asprintf (&topic, "%s@%s", type, element_src);
    mlm_client_send(client, topic, &msg);
    // To make sure message is send before we do something else
    // https://github.com/Malamute/malamute-core/issues/35
    zclock_sleep (100);

    zstr_free (&topic);
    mlm_client_destroy (&client);
    return 0;
}
