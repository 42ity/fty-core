/* 
Copyright (C) 2014 Eaton
 
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
Author: Karol Hrdina <karolhrdina@eaton.com>
 
Description: zeromq utilities
References: BIOS-248
*/
#ifndef UTILITIES_ZEROMQ_H_
#define UTILITIES_ZEROMQ_H_

#include <string>
#include <zmq.hpp>

namespace utils
{

namespace zeromq
{

/*!
\brief Convert zmq::message_t to std::string
\param[out] string
\param[in] message
*/
void zmsg_to_str(std::string& string, const zmq::message_t& message);

/*!
\brief Receive zeromq string from socket and convert it to a std::string
\param[in] sock zeromq socket from which to receive
\param[out] string received zmq::message_t converted to string
*/
void str_recv(zmq::socket_t& socket, std::string& string);

/*!
\brief Convert std::string to zmq::message_t and send it to a socket
\param[in] socket socket to which to send the string
\param[in] string string to be sent
\return true if successfull, false otherwise
*/
bool str_send(zmq::socket_t& socket, const std::string& string);

/*!
\brief Convert std::string to zmq::message_t and send it to a socket
        as multipart, non-terminal

A sequence of these calls followed by str_send() allows to send one
multipart message.

\param[in] socket socket to which to send the string
\param[in] string string to be sent
\return true if successfull, false otherwise
*/
bool str_sendmore(zmq::socket_t& socket, const std::string& string);

} // namespace zmq

} // namespace utils

#endif // UTILITIES_ZEROMQ_H_

