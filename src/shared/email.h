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

/*! \file email.h
    \brief Simple wrapper on top of msmtp to send an email
    \author Michal Vyskocil <michalvyskocil@eaton.com>

Example:

*/
#include "subprocess.h"
#include <string>

#ifndef _SRC_SHARED_EMAIL_H
#define _SRC_SHARED_EMAIL_H
namespace shared {
    class Smtp
    {
        public:
            explicit Smtp(
                    const std::string& host,
                    const std::string& from
                    );
     

            void sendmail(
                    const std::string& to,
                    const std::string& subject,
                    const std::string& body);

            void sendmail(
                    const std::string& body);

        protected:

            std::string _host;
            std::string _from;
            Argv _argv;
    };
};

#endif // _SRC_SHARED_EMAIL_H
