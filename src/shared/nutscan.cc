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

int
nut_scan_snmp(
        const char* name,
        const char* ip_address,
        std::string& out)
{
    Argv args = {"/usr/bin/nut-scanner", "-S", "-s", ip_address, "-e", ip_address};
    std::string e;

    int ret = output(args, out, e);
    if (ret != 0)
        return -1;

    if (out.empty())
        return -1;

    auto idx = out.find("[");
    if (idx == std::string::npos)
        return -1;

    out.erase(0, idx-1);
    out.replace(2, 7, name);
    return 0;
}

}
