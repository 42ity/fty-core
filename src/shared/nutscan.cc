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

#include <sstream>

#include "nutscan.h"
#include "subprocess.h"
#include "log.h"

namespace shared {

//MVY: copy&paste from nut/tools/nut-scanner/nutscan-display.c
//     update to push things to the std::stringstream
static void
s_nutscan_display_ups_conf(
        const std::string& name,
        nutscan_device_t * device,
        std::vector<std::string>& ret)
{
	nutscan_device_t * current_dev = device;
	nutscan_options_t * opt;
	static int nutdev_num = 1;

	if(device==NULL) {
		return;
	}

	/* Find start of the list */
	while(current_dev->prev != NULL) {
		current_dev = current_dev->prev;
	}

	/* Display each devices */
	do {
        std::stringstream out;
        out << "["  << name << "]" << std::endl;
        out << "\tdriver = \"" << current_dev->driver  << "\"" << std::endl;
        out << "\tport = \""   << current_dev->port    << "\"" << std::endl;

		opt = current_dev->opt;

		while (NULL != opt) {
			if( opt->option != NULL ) {
                out << "\t" << opt->option;
				if( opt->value != NULL ) {
                    out << " = \"" << opt->value << "\"";
				}
				out << std::endl;
			}
			opt = opt->next;
		}

		nutdev_num++;

		current_dev = current_dev->next;
        ret.push_back(out.str());
	}
	while( current_dev != NULL );
}

int
nut_scan_snmp(
        const std::string& name,
        const CIDRAddress& ip_address,
        nutscan_snmp_t* snmp_conf,
        std::vector<std::string>& out)
{

    static nutscan_snmp_t s_snmp_conf;
	memset(&s_snmp_conf, 0, sizeof(s_snmp_conf));

    auto sip = ip_address.toString();
    auto dev = nutscan_scan_snmp(
            sip.c_str(),
            sip.c_str(),
            5000000,
            snmp_conf ? snmp_conf : &s_snmp_conf);

    s_nutscan_display_ups_conf(name, dev, out);
    nutscan_free_device(dev);

    if (out.empty())
        return -1;

    return 0;
}

}
