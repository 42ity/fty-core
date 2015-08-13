/*
Copyright (C) 2014 Eaton
 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/*! \file    test-libbiosapi.c
    \brief   CI script to validate libbiosapi
    \author  Michal Vyskocil <MichalVyskocil@eaton.com>
    \details This program is compiled by the CI scripts as a way to
             validate that the "-lbiosapi" linker flag and headers do still provide
             all that we need for an API-client program to work in the $BIOS project,
             and that some basic message-passing works as expected.
*/

#include <bios_agent.h>
#include <stdio.h>

#include "cleanup.h"

int main() {

    int r;

    static const char* endpoint = "ipc://@/malamute";
    static const char* content_type = "application/octet-stream";

    _scoped_bios_agent_t *reader = bios_agent_new(endpoint, "reader");
    assert(reader);
    fprintf(stderr, ">>>>>> reader::new done\n");
    
    _scoped_bios_agent_t *writer = bios_agent_new(endpoint, "writer");
    assert(writer);
    fprintf(stderr, ">>>>>> writer::new done\n");

    {
    _scoped_ymsg_t *ymsg = ymsg_new(YMSG_SEND);
    ymsg_set_content_type(ymsg, content_type);

    fprintf(stderr, ">>>>>> writer::sendto starting\n");
    r = bios_agent_sendto(writer, "reader", "test-subject", &ymsg);
    fprintf(stderr, ">>>>>> writer::sendto done, r:%d\n", r);
    assert(r == 0);
    }

    {
    fprintf(stderr, ">>>>>> reader::recv starting\n");
    _scoped_ymsg_t *ymsg = bios_agent_recv(reader);
    fprintf(stderr, ">>>>>> reader::reader done\n");
    const char* mime = ymsg_content_type(ymsg);
    assert(streq(mime, content_type));
    ymsg_destroy(&ymsg);
    }

    bios_agent_destroy(&writer);
    bios_agent_destroy(&reader);

}
