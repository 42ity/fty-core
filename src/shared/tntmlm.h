/*
 *
 * Copyright (C) 2016 Eaton
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

#ifndef SRC_SHARED_TNTMLM_H
#define SRC_SHARED_TNTMLM_H

// Original idea of using cxxtools::Pool of mlm_client_t* connections
// by Michal Hrusecky <michal@hrusecky.net> 

#include <memory>
#include <string>

#include <cxxtools/pool.h>
#include <malamute.h>

class MlmClient {
    public:
        static const std::string ENDPOINT;  

        MlmClient ();
        ~MlmClient ();

        // timeout <0, 300> seconds, greater number trimmed
        // based on specified uuid returns expected message or NULL on expire/interrupt
        zmsg_t*     recv (const std::string& uuid, uint32_t timeout);
        int         sendto (const std::string& address,
                            const std::string& subject,
                            uint32_t timeout,
                            zmsg_t **content_p);
        bool        connected () { return mlm_client_connected (_client); }
        const char* subject () { return mlm_client_subject (_client); }
        const char* sender () { return mlm_client_sender (_client); }

    private:
        void connect ();
        mlm_client_t*   _client;
        zuuid_t*        _uuid;
        zpoller_t*      _poller;
};

typedef cxxtools::Pool <MlmClient> MlmClientPool;

extern MlmClientPool mlm_pool;

#endif // SRC_SHARED_TNTMLM_H

