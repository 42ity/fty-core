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
#include "utilities_zeromq.h"

namespace utils
{

namespace zeromq
{

void zmsg_to_str(std::string& string, const zmq::message_t& message) {
  string.assign(static_cast<const char *>(message.data()), message.size());
}

void str_recv(zmq::socket_t& socket, std::string& string) {
  zmq::message_t msg;
  socket.recv(&msg);
  string.assign(static_cast<char *>(msg.data()), msg.size());
}

bool str_send(zmq::socket_t& socket, const std::string& string) {
  zmq::message_t msg(string.size());
  memcpy(msg.data(), string.data(), string.size()); // TODO KHR look into zmq c api
  bool ret = socket.send(msg);
  return ret;
}

bool str_sendmore(zmq::socket_t& socket, const std::string& string) {
  zmq::message_t msg(string.size());
  memcpy(msg.data(), string.data(), string.size());
  bool ret = socket.send(msg, ZMQ_SNDMORE);
  return ret;
}

} // namespace zmq

} // namespace utils
