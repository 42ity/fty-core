/*
Copyright (C) 2014 - 2015 Eaton

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
 \file   agents.h
 \brief  TODO
 \author Karol Hrdina <KarolHrdina@eaton.com>
*/

#ifndef INCLUDE_AGENTS__
#define INCLUDE_AGENTS__

#include <czmq.h>
#include <malamute.h>
#include <string.h>

#include "ymsg.h"

#ifdef __cplusplus
extern "C" {
#endif

/*    
// For certain items, contradict the default of GCC "-fvisibility=hidden"
#ifndef BIOS_EXPORT
# if BUILDING_LIBBIOSAPI && HAVE_VISIBILITY
#  define BIOS_EXPORT __attribute__((__visibility__("default")))
# else
#  define BIOS_EXPORT
# endif
#endif
*/

BIOS_EXPORT ymsg_t *
    bios_netmon_encode (int event, const char *interface_name, int ip_version, const char *ip_address, uint8_t prefix_length, const char *mac_address);


// on -1 (error) does not destroy *self_p
BIOS_EXPORT int
    bios_netmon_decode (ymsg_t **self_p, int *event, char *interface_name, int *ip_version, char *ip_address, uint8_t *prefix_length, char *mac_address);

#ifdef __cplusplus
}
#endif

#endif // INCLUDE_AGENTS__

