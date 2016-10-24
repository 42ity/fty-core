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

#include "log.h"
#include "tntmlm.h"

MlmClientPool mlm_pool {10};

const std::string MlmClient::ENDPOINT = "ipc://@/malamute";

MlmClient::MlmClient ()
{
    _client = mlm_client_new ();
    _uuid = zuuid_new ();
    _poller = zpoller_new (mlm_client_msgpipe (_client), NULL);
    connect ();
}

MlmClient::~MlmClient ()
{
    zuuid_destroy (&_uuid);
    zpoller_destroy (&_poller);
    mlm_client_destroy (&_client);
}

zmsg_t*
MlmClient::recv (const std::string& uuid, uint32_t timeout)
{
    if (!connected ()) {
        connect ();
    }
    uint64_t wait = timeout;
    if (wait > 300) {
        wait = 300;
    }

    uint64_t start = static_cast <uint64_t> (zclock_mono ());
    uint64_t poller_timeout = wait * 1000;  

    while (true) {
        void *which = zpoller_wait (_poller, poller_timeout);
        if (which == NULL) {
            log_warning (
                    "zpoller_wait (timeout = '%" PRIu64"') returned NULL. zpoller_expired == '%s', zpoller_terminated == '%s'",
                    poller_timeout,
                    zpoller_expired (_poller) ? "true" : "false",
                    zpoller_terminated (_poller) ? "true" : "false");
            return NULL;
        }
        uint64_t timestamp = static_cast <uint64_t> (zclock_mono ());
        if (timestamp - start >= poller_timeout) {
            poller_timeout = 0;
        }
        else {
            poller_timeout = poller_timeout - (timestamp - start);
        }
        zmsg_t *msg = mlm_client_recv (_client);
        if (!msg)
            continue;
        char *uuid_recv = zmsg_popstr (msg);
        if (uuid_recv && uuid.compare (uuid_recv) == 0) {
            zstr_free (&uuid_recv);
            return msg;
        }
        zstr_free (&uuid_recv);
        zmsg_destroy (&msg);
    }
}

int
MlmClient::sendto (const std::string& address,
                   const std::string& subject, 
                   uint32_t timeout, 
                   zmsg_t **content_p)
{
    if (!connected ()) {
        connect ();
    }
    return mlm_client_sendto (_client, address.c_str (), subject.c_str (), NULL, timeout, content_p);
}

void
MlmClient::connect ()
{
    std::string name ("web.");
    name.append (zuuid_str_canonical (_uuid));
    int rv = mlm_client_connect (_client, ENDPOINT.c_str (), 5000, name.c_str ());
    if (rv == -1) {
        log_error (
                "mlm_client_connect (endpoint = '%s', timeout = 5000, address = '%s') failed",
                ENDPOINT.c_str (), name.c_str ());
    }
}


