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

/*
 * nut-scanner wrapper
 */

#include "subprocess.h"

namespace shared {

std::string
nut_scan_snmp(
        const char* name,
        const char* ip_address)
{
    Argv args = {"/usr/bin/nut-scanner", "-S", "-s", ip_address, "-e", ip_address};
    std::string o;
    std::string e;

    int ret = output(args, o, e);
    if (ret != 0)
        return "";

    if (o.empty())
        return "";

    auto idx = o.find("[");
    if (idx == std::string::npos)
        return "";

    o.erase(0, idx-1);
    o.replace(2, 7, name);
    return o;
}

}
