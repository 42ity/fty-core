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

#include "email.h"
#include "log.h"
#include <sstream>

namespace shared {

Smtp::Smtp(
        const std::string& host,
        const std::string& from
        ) :
    _host{host},
    _from{from}
{
    _argv = Argv{ \
        "/usr/bin/msmtp",
        "--host=" + _host,
        "--protocol=smtp",
        "--auth=off",
        "--tls=off",
        "--auto-from=on",
        "--read-recipients",
        "--from=" + _from};
}

void Smtp::sendmail(
        const std::string& to,
        const std::string& subject,
        const std::string& body)
{
    std::ostringstream sbuf;

    sbuf << "To: ";
    sbuf << to;
    sbuf << "\n";

    sbuf << "Subject: ";
    sbuf << subject;
    sbuf << "\n";

    sbuf << body;
    sbuf << "\n";

    return sendmail(sbuf.str());
}

void Smtp::sendmail(
        const std::string& body)
{
    SubProcess proc{_argv, SubProcess::STDIN_PIPE | SubProcess::STDOUT_PIPE | SubProcess::STDERR_PIPE};

    bool bret = proc.run();
    if (!bret) {
        throw std::runtime_error( \
                "/usr/bin/msmtp failed with exit code '" + \
                std::to_string(proc.getReturnCode()) + "'\nstderr:\n" + \
                read_all(proc.getStderr()));
    }

    ssize_t wr = ::write(proc.getStdin(), body.c_str(), body.size());
    if (wr != static_cast<ssize_t>(body.size())) {
        log_warning("Email truncated, exp '%zu', piped '%zd'", body.size(), wr);
    }
    ::close(proc.getStdin()); //EOF

    bret = proc.wait();
    int ret = proc.getReturnCode();
    if (ret != 0) {
        throw std::runtime_error( \
                "/usr/bin/msmtp failed with exit code '" + \
                std::to_string(proc.getReturnCode()) + "'\nstderr:\n" + \
                read_all(proc.getStderr()));
    }

}

};
