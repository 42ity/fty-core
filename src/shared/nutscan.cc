/*
Copyright (C) 2015 Eaton
 
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

#include <string>
#include <sstream>
#include <vector>

#include "preproc.h"
#include "nutscan.h"
#include "subprocess.h"
#include "log.h"

namespace shared {

/**
 * \brief one read line from istream
 */
static
ssize_t
s_readline(
        std::istream& inp,
        std::string& line)
{
    std::ostringstream buf;
    buf.clear();
    std::size_t ret;
    if (!inp.good() || inp.eof()) {
        return -1;
    }

    for (ret = 0; inp.good() && inp.peek() != '\n'; ret++) {
        char ch = static_cast<char>(inp.get());
        if (inp.eof())
            break;
        buf << ch;
    }
    if (inp.peek() == '\n') {
        buf << static_cast<char>(inp.get());
    }

    line.append(buf.str());
    return ret;
}

/**
 * \brief parse an output of nut-scanner
 *
 * \param name - name of device in the result
 * \param inp  - input stream
 * \param out  - vector of string with output
 */
static
void
s_parse_nut_scanner_output(
        const std::string& name,
        std::istream& inp,
        std::vector<std::string>& out)
{

    if (!inp.good() || inp.eof())
        return;

    std::stringstream buf;

    while (inp.good() && !inp.eof()) {
        std::string line;

        if (s_readline(inp, line) == -1)
            break;

        if (line.size() == 0)
            continue;

        if (line[0] == '[') {
            if (buf.tellp() > 0) {
                out.push_back(buf.str());
                buf.clear();
                buf.str("");
            }
            buf << '[' << name << ']' << std::endl;
        }
        else if (buf.tellp() > 0) {
            buf << line;
        }
    }

    if (buf.tellp() > 0) {
        out.push_back(buf.str());
        buf.clear();
        buf.str("");
    }
}

/**
 * \brief run nut-scanner binary and return the output
 */
static
int
s_run_nut_scaner(
        const shared::Argv& args,
        const std::string& name,
        std::vector<std::string>& out)
{
    std::string o;
    std::string e;
    int ret = shared::output(args, o, e, 10);

    if (ret != 0)
        return -1;

    std::istringstream inp{o};
    s_parse_nut_scanner_output(
            name,
            inp,
            out);

    if (out.empty())
        return -1;

    return 0;
}

int
nut_scan_snmp(
        const std::string& name,
        const CIDRAddress& ip_address,
        std::vector<std::string>& out)
{
    shared::Argv args = {"nut-scanner", "-S", "-s", ip_address.toString()};
    return s_run_nut_scaner(
            args,
            name,
            out);
}


int
nut_scan_xml_http(
        UNUSED_PARAM const std::string& name,
        UNUSED_PARAM const CIDRAddress& ip_address,
        UNUSED_PARAM std::vector<std::string>& out)
{
#ifndef HAVE_ETN_NUTSCAN_SCAN_XML_HTTP
    return -1; // not supported
#else
    shared::Argv args = {"nut-scanner", "-M", "-s", ip_address.toString()};
    return s_run_nut_scaner(
            args,
            name,
            out);
#endif // HAVE_ETN_NUTSCAN_SCAN_XML_HTTP
}

}
