/*
Copyright (C) 2015 Eaton

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*!
 \file   cm-agent-web.cc
 \brief  TODO
 \author Karol Hrdina <KarolHrdina@eaton.com>
*/

#include "bios_agent.h"
#include "agents.h"
#include "defs.h"
#include "log.h"
#include "str_defs.h"
#include "utils.h"
#include "utils_ymsg.h"
#include "utils_ymsg++.h"

#include "cm-agent-web.h"

int
process_web_average
(bios_agent_t *agent, ymsg_t *message_in, std::map <uint16_t, std::pair <std::string, ymsg_t *>>& requests, const char *sender, ymsg_t **message_out) {
    if (!agent || !sender || !message_in || !message_out)
        return -1;

    // average request decode variables
    int64_t start_ts = -1, end_ts = -1;
    char *type = NULL, *step = NULL, *source = NULL;
    uint64_t element_id = 0;

    int rv = bios_web_average_request_extract (message_in, &start_ts, &end_ts, &type, &step, &element_id, &source);
    if (rv != 0) {
        log_error ("bios_web_average_request_extract () failed.");

        ymsg_t *msg_reply = ymsg_new (YMSG_REPLY);
        assert (msg_reply);
        ymsg_set_status (msg_reply, false);
        ymsg_set_errmsg (msg_reply, "Internal error. Please check logs."); // TODO: maybe rewrite the message

        *message_out = msg_reply;
        return 0;       
    }
    // Prepare measurements read request
    char *send_subject = NULL;
    // When user says, e.g.: "I want 8h averages starting from 23:15:.... " we need to request sampled data from an earlier period
    // in order to compute the first 8h average. This first average belongs to 24:00:00 and is computed from <16:00, 24:00).
    // Additional AGENT_NUT_REPEAT_INTERVAL_SEC seconds are deducted to complete possible leading values `missing`.
    start_ts = average_extend_left_margin (start_ts, step); // shared/utils.c 
    // we are requesting sampled data from db, therefore no topic construction (i.e. <source>.<type>_<step>) is needed.
    ymsg_t *msg_send = bios_db_measurements_read_request_encode (start_ts, end_ts, element_id, source, &send_subject);
    FREE0 (type);
    FREE0 (step);
    FREE0 (source);
    assert (msg_send);  

    // 'seq' of the next outcoming message
    uint16_t seq = bios_agent_seq (agent);
    std::string msg_print;
    ymsg_format (msg_send, msg_print);
    // send the request to db-ng
    rv = bios_agent_sendto (agent, BIOS_AGENT_NAME_DB_MEASUREMENT, send_subject, &msg_send); // msg_send is destroyed
    FREE0 (send_subject);
    if (rv != 0) {
        log_critical ("bios_agent_sendto (\"%s\", \"%s\") failed.", BIOS_AGENT_NAME_DB_MEASUREMENT, send_subject);

        ymsg_t *msg_reply = ymsg_new (YMSG_REPLY);
        assert (msg_reply);
        ymsg_set_status (msg_reply, false);
        ymsg_set_errmsg (msg_reply, "Internal error. Please check logs."); // TODO: maybe rewrite the message

        *message_out = msg_reply;
        return 0;
    }
    requests.emplace (std::make_pair (seq, std::make_pair (std::string(sender), ymsg_dup (message_in)))); // msg_recv is saved
    log_debug ("ACTION: Message sent to %s\n%s", BIOS_AGENT_NAME_DB_MEASUREMENT, msg_print.c_str ());
    return 0;
}
