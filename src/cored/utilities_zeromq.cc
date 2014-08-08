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
  memcpy(msg.data(), string.data(), string.size()); // :/
  bool ret = socket.send(msg, ZMQ_SNDMORE);
  return ret;
}

} // namespace zmq

} // namespace utils
