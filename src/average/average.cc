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
 * \file average.cc
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 * \author Jim Klimov <EvgenyKlimov@Eaton.com>
 * \brief  Implementation of average/min/max calculation
 */

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <ctime>
#include <cxxtools/split.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <tnt/tntnet.h>

#include "utils.h"
#include "utils++.h"
#include "bios_agent.h"
#include "agents.h"
#include "ymsg.h"
#include "log.h"
#include "str_defs.h"
#include "utils_ymsg.h"
#include "cleanup.h"
#include "utils_web.h"
#include "helpers.h"

// Check if value of parameter 'relative' is supported:
//  * No - return false, value of unixtime is not changed
//  * Yes - return true; value of unixtime contains now - (relative expressed in seconds)
//
// Currently supported values of relative: 24h, 7d, 30d
// Expects string relative converted to lowercase.

bool relative_to_unixtime (const std::string& relative, int64_t now, int64_t& unixtime) {
    if (relative.compare ("24h") == 0) {
        unixtime = now - 86400;
        return true;
    }
    else if (relative.compare ("7d") == 0) {
        unixtime = now - 604800;
        return true;
    }
    else if (relative.compare ("30d") == 0) {
        unixtime = now - 2592000;
        return true;
    }
    return false;
}

class StdReply {
    public:
        std::ostream & out() { return std::cerr; }
};

int main(int argc, char **argv) {

    log_open ();
    if (argc == 1) {
        log_error ("Usage: %s query", argv[0]);
        exit (EXIT_FAILURE);
    }

    tnt::QueryParams q {};
    q.parse_url (argv[1]);

    std::string start_ts = q["start_ts"];
    std::string end_ts = q["end_ts"];
    std::string type = q["type"];
    std::string step = q["step"];
    std::string source = q["source"];
    std::string element_id = q["element_id"];
    std::string relative = q["relative"];

    auto reply = StdReply{};

    // Input argument checking
    int64_t st = -1, end = -1;
    if (relative.empty ()) {
        // check both start_ts and end_ts
        if (start_ts.empty ()) {
            http_die ("request-param-required", "start_ts");
        }
        st = datetime_to_calendar (start_ts.c_str ());
        if (st == -1) {
            http_die ("request-param-bad", "start_ts", std::string ("'").append (start_ts).append ("'").c_str (), "format 'YYYYMMDDhhmmssZ");
        }
        if (end_ts.empty ()) {
            http_die ("request-param-required", "end_ts");
        }
        end = datetime_to_calendar (end_ts.c_str ());
        if (end == -1) {
            http_die ("request-param-bad", "end_ts", std::string ("'").append (end_ts).append ("'").c_str (), "format 'YYYYMMDDhhmmssZ");
        }

        // check that start_ts < end_ts
        if (end <= st) {
            http_die ("parameter-conflict",
                std::string ("Start timestamp '").append (start_ts).append ("' is greater than end timestamp '").append (end_ts).append ("'.").c_str ());
        }
    }
    else {
        // check value of relative parameter
        std::transform (relative.begin(), relative.end(), relative.begin(), ::tolower);
        int64_t now = (int64_t) time (NULL); 
        if (!relative_to_unixtime (relative, now, st)) {
            http_die ("request-param-bad", "relative", std::string ("'").append (relative).append ("'").c_str (),
                      "one of the following values: '24h', '7d', '30d'.");
        } 
        end = (int64_t) time (NULL);
    }

    log_debug ("st = '%" PRIi64"', end = '%" PRIi64"'", st, end);

    // type is optional, default type is arithmetic average
    if (type.empty ()) {
        type.assign (AVG_TYPES[0]);
    }
    else if (!is_average_type_supported (type.c_str ())) {
        http_die ("request-param-bad", "type",
            std::string ("'").append (type).append ("'").c_str (),
            std::string ("one of the following values: [").append (utils::join (AVG_TYPES, AVG_TYPES_SIZE, ", ")).append("].").c_str ());
    }

    // step 
    if (step.empty ()) {
        http_die ("request-param-required", "step");
    } 
    if (!is_average_step_supported (step.c_str ())) {
        http_die ("request-param-bad", "step",
            std::string ("'").append (step).append ("'").c_str (),
            std::string ("one of the following values: [").append (utils::join (AVG_STEPS, AVG_STEPS_SIZE, ", ")).append("].").c_str ());
    }

    // source
    if (source.empty()) {
        http_die ("request-param-required", "source");
    }

    // element_id
    http_errors_t errors;
    uint32_t eid = 0;

    if (!check_element_identifier ("element_id", element_id, eid, errors)) {
        http_die_error (errors);
    }

    std::string agent_name (BIOS_AGENT_PREFIX_REST);
    agent_name.append (std::to_string (static_cast<int> (getpid ()))).append (".").append (std::to_string ( syscall(SYS_gettid) ));

    _scoped_bios_agent_t *agent = bios_agent_new (MLM_ENDPOINT, agent_name.c_str ());
    if (agent == NULL) {
        http_die ("internal-error", "bios_agent_new () failed.");
    }

    _scoped_ymsg_t *msg = bios_web_average_request_encode (st, end, type.c_str(), step.c_str(), eid, source.c_str ());
    if (msg == NULL) {
        bios_agent_destroy (&agent);
        http_die ("internal-error", "bios_web_average_request_encode() failed.");
    }

    int rv = bios_agent_sendto (agent, BIOS_AGENT_NAME_COMPUTATION, "metric/computed/average", &msg);
    if (rv != 0) {
        bios_agent_destroy (&agent);
        http_die ("internal-error", "bios_web_average_send_to() failed.");
    }

    msg = bios_agent_recv (agent);
    if (msg == NULL) {
        bios_agent_destroy (&agent);
        http_die ("internal-error", "Computation module didn't reply.");
    }

    if (!ymsg_is_ok (msg)) {
        ymsg_destroy (&msg);
        bios_agent_destroy (&agent);
        http_die ("internal-error", ymsg_errmsg (msg) ? ymsg_errmsg (msg) : "Error requesting average data." );
    }

    _scoped_char *json = NULL;
    rv = bios_web_average_reply_extract (msg, &json);
    ymsg_destroy (&msg);
    if (rv != 0) {
        bios_agent_destroy (&agent);
        http_die ("internal-error", "Reply from computation module incorrect.");
    }
    bios_agent_destroy (&agent);

    log_debug (json);
    zstr_free (&json);

}
