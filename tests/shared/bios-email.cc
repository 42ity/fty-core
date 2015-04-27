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

/*! \file bios-email.cc
    \brief Simple command line client as an example on how Smtp class should be used
    \author Michal Vyskocil <michalvyskocil@eaton.com>
*/
#include "log.h"
#include "subprocess.h"
#include "sendmail.h"

int main(int argc, char** argv) {

    if (argc != 4) {
        log_error("Usage: ./bios-email To Subject Body");
        return -1;
    }

    shared::Smtp smtp{"mail.etn.com", "bios-test@no.ip"};

    try {
        smtp.sendmail(
            argv[1],
            argv[2],
            argv[3]
        );
    }
    catch (const std::runtime_error& e) {
        log_error(e.what());
        return -1;
    };

    return 0;
}
