/*
 #
 # Copyright (C) 2015 Eaton
 #
 # This program is free software; you can redistribute it and/or modify
 # it under the terms of the GNU General Public License as published by
 # the Free Software Foundation; either version 2 of the License, or
 # (at your option) any later version.
 #
 # This program is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 # GNU General Public License for more details.
 #
 # You should have received a copy of the GNU General Public License along
 # with this program; if not, write to the Free Software Foundation, Inc.,
 # 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 #
 #
*/
/*!
 * \file warranty-metric.cc
 * \author Michal Vyskocil <MichalVyskocil@Eaton.com>
 * \brief  Send a warranty metric for all elements with element_date in the DB
 */

#include <biosproto.h>
#include <tntdb.h>

#include "log.h"
#include "dbpath.h"
#include "db/assets.h"

/*
 * Tool will send following messages on the stream METRICS
 *
 *  SUBJECT: end_warranty_date@device and warranty_expiration_date@device
 *           value ISO date
 */

int main()
{
    log_open ();

    mlm_client_t *client = mlm_client_new ();
    assert (client);

    std::function<void(const tntdb::Row&)> cb = \
        [client](const tntdb::Row &row)
        {
            std::string name;
            row["name"].get(name);

            std::string keytag;
            row["keytag"].get(keytag);

            std::string date;
            row["date"].get(date);

            //TODO
            log_debug ("name: %s, keytag: %s, date: %s", name.c_str(), keytag.c_str(), date.c_str());
            zmsg_t *msg = bios_proto_encode_metric (
                    NULL,
                    keytag.c_str(),
                    name.c_str (),
                    date.c_str (),
                    "day",
                    -1);
            assert (msg);
            std::string subject = keytag.append ("@").append (name);
            mlm_client_send (client, subject.c_str (), &msg);
        };

    int r = mlm_client_connect (client, "ipc://@/malamute", 1000, "warranty-metric");
    if (r == -1) {
        log_error ("Can't connect to malamute");
        exit (EXIT_FAILURE);
    }

    r = mlm_client_set_producer (client, "METRICS");
    if (r == -1) {
        log_error ("Can't set producer to METRICS stream");
        exit (EXIT_FAILURE);
    }

    // unchecked errors with connection, the tool will fail otherwise
    tntdb::Connection conn = tntdb::connectCached(url);
    r = persist::select_asset_element_all_with_warranty_end (conn, cb);
    if (r == -1) {
        log_error ("Error in element selection");
        exit (EXIT_FAILURE);
    }

    // to ensure all messages got published
    zclock_sleep (500);
    mlm_client_destroy (&client);

    exit (EXIT_SUCCESS);
}
