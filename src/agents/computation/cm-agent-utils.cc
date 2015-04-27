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
 \file   cm-agent-utils.cc
 \brief  TODO
 \author Karol Hrdina <KarolHrdina@eaton.com>
*/
#include <string>

#include "log.h"
#include "utils_ymsg.h"
#include "utils_ymsg++.h"

#include "cm-agent-utils.h"


int 
replyto_err 
(bios_agent_t *agent, ymsg_t **original, const char *sender, const char *error_message, const char *subject) {
    if (!original || !*original)
        return -1;
    if (!agent || !sender || !error_message || !subject) {
        ymsg_destroy (original);
        return -1;
    }

    ymsg_t *msg_reply = ymsg_new (YMSG_REPLY);
    assert (msg_reply);
    ymsg_set_status (msg_reply, false);
    ymsg_set_errmsg (msg_reply, error_message);
    std::string formatted_message;
    ymsg_format (msg_reply, formatted_message);
    int rv = bios_agent_replyto (agent, sender, subject, &msg_reply, *original); // msg_reply is destroyed
    ymsg_destroy (original); // original is destroyed
    if (rv != 0) {
        log_critical ("bios_agent_replyto (\"%s\", \"%s\") failed.", sender, subject);
    }
    else {
        log_debug ("ACTION. Error message sent to %s with subject %s:\n%s", sender, subject, formatted_message.c_str ());
    }
    return 0;
}

