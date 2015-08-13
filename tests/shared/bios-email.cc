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

/*! \file   bios-email.cc
    \brief  Simple command line client as an example on how Smtp class should be used
    \author Michal Vyskocil <michalvyskocil@eaton.com>
*/
#include "log.h"
#include "subprocess.h"
#include "email.h"

int main(int argc, char** argv) {

    if (argc != 5) {
        log_error("Usage: ./bios-email From To Subject Body");
        return -1;
    }

    try {
        shared::Smtp smtp{"mail.etn.com", argv[1]};

        smtp.sendmail(
            argv[2],
            argv[3],
            argv[4]
        );
    }
    catch (const std::runtime_error& e) {
        log_error(e.what());
        return -1;
    };

    return 0;
}
