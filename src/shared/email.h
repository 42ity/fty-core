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

/*! \file   email.h
    \brief  Simple wrapper on top of msmtp to send an email
    \author Michal Vyskocil <MichalVyskocil@Eaton.com>

Example:

    shared::Smtp smtp{"mail.example.com", "joe.doe@example.com"};

    try {
        smtp.sendmail(
            "agent.smith@matrix.gov",
            "The pill taken by Neo",
            "Dear Mr. Smith,\n......");
    }
    catch (const std::runtime_error& e) {
       // here we'll handle the error
    }

*/
#include "subprocess.h"
#include <string>

#ifndef _SRC_SHARED_EMAIL_H
#define _SRC_SHARED_EMAIL_H
namespace shared {
    /**
     * \class Smtp
     *
     * \brief Simple wrapper on top of msmtp
     *
     * This class contain some basic configuration for msmtp (host/from) + provide sendmail methods. It *DOES NOT* perform any additional transofmation like uuencode or mime. IOW garbage-in, garbage-out.
     *
     */
    class Smtp
    {
        public:
            /**
             * \brief Creates SMTP instance
             *
             * \param host   hostname of remote smtp server
             * \param from   envelope from
             */
            explicit Smtp(
                    const std::string& host,
                    const std::string& from
                    );

            /**
             * \brief send the email
             *
             * Technically this put email to msmtp's outgoing queue
             * \param to        email header To: multiple recipient in vector
             * \param subject   email header Subject:
             * \param body      email body
             *
             * \throws std::runtime_error for msmtp invocation errors
             */
            void sendmail(
                const std::vector<std::string> &to,
                const std::string& subject,
                const std::string& body);

            /**
             * \brief send the email
             *
             * Technically this put email to msmtp's outgoing queue
             * \param to        email header To: single recipient
             * \param subject   email header Subject:
             * \param body      email body
             *
             * \throws std::runtime_error for msmtp invocation errors
             */
            void sendmail(
                    const std::string& to,
                    const std::string& subject,
                    const std::string& body);

            /**
             * \brief send the email
             *
             * Technically this put email to msmtp's outgoing queue
             * \param data  email DATA (To/Subject are deduced
             *              from the fields in body, so body must be properly
             *              formatted email message).
             *
             * \throws std::runtime_error for msmtp invocation errors
             */
            void sendmail(
                    const std::string& data) const;

        protected:

            std::string _host;
            std::string _from;
            Argv _argv;
    };
};

#endif // _SRC_SHARED_EMAIL_H
