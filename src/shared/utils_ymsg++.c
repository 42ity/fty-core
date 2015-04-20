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
#include "utils_ymsg++.h"

void
ymsg_format (ymsg_t *self, std::string& str) {
    if (!self) {
        str.assign ("(NULL)");
        return;
    }

    str.append ("id=").append (ymsg_command (self)).append ("\n");
    str.append ("version=").append (std::to_string (ymsg_version (self))).append ("\n");
    str.append ("seq=").append (std::to_string (ymsg_seq (self))).append ("\n");
    if (ymsg_id (self) == YMSG_REPLY) {
        str.append ("rep=").append (std::to_string (ymsg_rep (self))).append ("\n");
    }
    str.append ("aux=");
    if (ymsg_aux (self)) {
        str.append ("\n");
        char *item = (char *) zhash_first (ymsg_aux (self));
        while (item) {
            str.append ("\t").append (zhash_cursor (ymsg_aux (self))).append ("=").append (item).append ("\n");
            item = (char *) zhash_next (ymsg_aux (self));
        }
    } else
        str.append ("(NULL)\n");

    str.append ("request=");
    if (ymsg_request (self)) {
        str.append (std::to_string (zchunk_size (ymsg_request (self)))).append (" bytes in size\n");
    }
    else
       str.append ("(NULL)\n");

    if (ymsg_id (self) == YMSG_REPLY) {
        str.append ("response=");
        if (ymsg_response (self)) {
            str.append (std::to_string (zchunk_size (ymsg_response (self)))).append (" bytes in size\n");
        }
        else
            str.append ("(NULL)\n");
    }
}

